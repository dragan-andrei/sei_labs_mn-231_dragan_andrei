#include "ctrl_task_sensor.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "configs.h"
#include "ctrl_stdio.h"
#include "dd_temp_sensor.h"

typedef struct
{
    float last_temperature_c;
    float raw_temperature_c;
    float salt_pepper_temperature_c;
    float weighted_temperature_c;
    uint16_t raw_adc_value;
    uint16_t salt_pepper_adc_value;
    uint16_t weighted_adc_value;
    uint8_t is_data_valid;
    uint8_t is_alert_active;
    uint32_t sample_count;
    dd_temp_sensor_status_t sensor_status;
} ctrl_sensor_signals_t;

static dd_temp_sensor_t g_temp_sensor;
static ctrl_sensor_signals_t g_sensor_signals;
static uint8_t g_sensor_module_initialized = CTRL_SENSOR_FLAG_DISABLED;

typedef enum
{
    CTRL_SENSOR_STATUS_TEXT_LONG = 0,
    CTRL_SENSOR_STATUS_TEXT_SHORT
} ctrl_sensor_status_text_variant_t;

static const char *ctrl_sensor_status_to_text(dd_temp_sensor_status_t status,
                                              ctrl_sensor_status_text_variant_t text_variant)
{
    if (text_variant == CTRL_SENSOR_STATUS_TEXT_SHORT)
    {
        switch (status)
        {
            case DD_TEMP_SENSOR_OK:
                return "OK";
            case DD_TEMP_SENSOR_DISCONNECTED:
                return "DISC";
            default:
                return "ERR";
        }
    }

    switch (status)
    {
        case DD_TEMP_SENSOR_OK:
            return "OK";
        case DD_TEMP_SENSOR_DISCONNECTED:
            return "DISCONNECTED";
        default:
            return "ERROR";
    }
}

static void ctrl_sensor_format_temperature(char *buffer, size_t buffer_size, float temperature_c)
{
    long scaled_value = (long)(temperature_c * SENSOR_TEMPERATURE_SCALE_FACTOR);
    long abs_scaled_value = (scaled_value < 0L) ? -scaled_value : scaled_value;
    long integer_part = abs_scaled_value / SENSOR_TEMPERATURE_SCALE_DIVISOR;
    long fractional_part = abs_scaled_value % SENSOR_TEMPERATURE_SCALE_DIVISOR;

    if (scaled_value < 0L)
    {
        snprintf(buffer, buffer_size, "-%ld.%02ld", integer_part, fractional_part);
    }
    else
    {
        snprintf(buffer, buffer_size, "%ld.%02ld", integer_part, fractional_part);
    }
}

static void ctrl_sensor_clear_filtered_channels(ctrl_sensor_signals_t *signals)
{
    signals->raw_temperature_c = CTRL_SENSOR_DEFAULT_TEMPERATURE_C;
    signals->salt_pepper_temperature_c = CTRL_SENSOR_DEFAULT_TEMPERATURE_C;
    signals->weighted_temperature_c = CTRL_SENSOR_DEFAULT_TEMPERATURE_C;
    signals->raw_adc_value = CTRL_SENSOR_DEFAULT_ADC_VALUE;
    signals->salt_pepper_adc_value = CTRL_SENSOR_DEFAULT_ADC_VALUE;
    signals->weighted_adc_value = CTRL_SENSOR_DEFAULT_ADC_VALUE;
}

static void ctrl_sensor_mark_invalid_sample(ctrl_sensor_signals_t *signals)
{
    signals->is_data_valid = CTRL_SENSOR_FLAG_DISABLED;
    signals->is_alert_active = CTRL_SENSOR_FLAG_DISABLED;
    ctrl_sensor_clear_filtered_channels(signals);
}

static void ctrl_sensor_update_alert_state(ctrl_sensor_signals_t *signals,
                                           float sampled_temperature)
{
    if (signals->is_alert_active == CTRL_SENSOR_FLAG_DISABLED)
    {
        if (sampled_temperature >= SENSOR_ALERT_THRESHOLD_C)
        {
            signals->is_alert_active = CTRL_SENSOR_FLAG_ENABLED;
        }
    }
    else
    {
        if (sampled_temperature <= (SENSOR_ALERT_THRESHOLD_C - SENSOR_ALERT_HYSTERESIS_C))
        {
            signals->is_alert_active = CTRL_SENSOR_FLAG_DISABLED;
        }
    }
}

static void ctrl_sensor_apply_new_sample(ctrl_sensor_signals_t *signals,
                                         const dd_temp_sensor_t *sensor,
                                         float sampled_temperature)
{
    signals->last_temperature_c = sampled_temperature;
    signals->raw_temperature_c = sensor->raw_temperature_c;
    signals->salt_pepper_temperature_c = sensor->salt_pepper_temperature_c;
    signals->weighted_temperature_c = sampled_temperature;
    signals->raw_adc_value = sensor->raw_adc_value;
    signals->salt_pepper_adc_value = sensor->salt_pepper_adc_value;
    signals->weighted_adc_value = sensor->last_adc_value;
    signals->is_data_valid = CTRL_SENSOR_FLAG_ENABLED;
    signals->sample_count++;

    ctrl_sensor_update_alert_state(signals, sampled_temperature);
}

static void ctrl_sensor_format_snapshot_temperatures(const ctrl_sensor_signals_t *snapshot,
                                                     char *temperature_text,
                                                     char *raw_temperature_text,
                                                     char *salt_pepper_temperature_text,
                                                     char *weighted_temperature_text,
                                                     size_t buffer_size)
{
    ctrl_sensor_format_temperature(temperature_text,
                                   buffer_size,
                                   snapshot->last_temperature_c);

    ctrl_sensor_format_temperature(raw_temperature_text,
                                   buffer_size,
                                   snapshot->raw_temperature_c);

    ctrl_sensor_format_temperature(salt_pepper_temperature_text,
                                   buffer_size,
                                   snapshot->salt_pepper_temperature_c);

    ctrl_sensor_format_temperature(weighted_temperature_text,
                                   buffer_size,
                                   snapshot->weighted_temperature_c);
}

static void ctrl_sensor_print_serial_reports(const ctrl_sensor_signals_t *snapshot,
                                             const char *temperature_text,
                                             const char *raw_temperature_text,
                                             const char *salt_pepper_temperature_text,
                                             const char *weighted_temperature_text)
{
    printf("[SENSOR] T=%s C | status=%s | valid=%u | alert=%u | samples=%lu\n",
           temperature_text,
           ctrl_sensor_status_to_text(snapshot->sensor_status, CTRL_SENSOR_STATUS_TEXT_LONG),
           snapshot->is_data_valid,
           snapshot->is_alert_active,
           (unsigned long)snapshot->sample_count);

    printf("[FILTER] adc_raw=%u(%s C) | adc_sare_piper=%u(%s C) | adc_medie_pond=%u(%s C)\n",
           snapshot->raw_adc_value,
           raw_temperature_text,
           snapshot->salt_pepper_adc_value,
           salt_pepper_temperature_text,
           snapshot->weighted_adc_value,
           weighted_temperature_text);
}

static void ctrl_sensor_build_lcd_lines(const ctrl_sensor_signals_t *snapshot,
                                        const char *temperature_text,
                                        char *lcd_line_1,
                                        char *lcd_line_2,
                                        size_t buffer_size)
{
    snprintf(lcd_line_1,
             buffer_size,
             "T:%sC %s",
             temperature_text,
             ctrl_sensor_status_to_text(snapshot->sensor_status, CTRL_SENSOR_STATUS_TEXT_SHORT));

    snprintf(lcd_line_2,
             buffer_size,
             "AL:%c N:%lu",
             snapshot->is_alert_active ? 'Y' : 'N',
             (unsigned long)snapshot->sample_count);
}

void ctrl_sensor_task_init(void)
{
    if (g_sensor_module_initialized != CTRL_SENSOR_FLAG_DISABLED)
    {
        return;
    }

    dd_temp_sensor_init(&g_temp_sensor, TEMP_SENSOR_PIN);

    g_sensor_signals.last_temperature_c = CTRL_SENSOR_DEFAULT_TEMPERATURE_C;
    ctrl_sensor_mark_invalid_sample(&g_sensor_signals);
    g_sensor_signals.sample_count = CTRL_SENSOR_DEFAULT_SAMPLE_COUNT;
    g_sensor_signals.sensor_status = dd_temp_sensor_get_status(&g_temp_sensor);

    g_sensor_module_initialized = CTRL_SENSOR_FLAG_ENABLED;
}

void ctrl_sensor_acquisition_task(void *params)
{
    (void)params;

    ctrl_sensor_task_init();

    vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_OFFSET_MS));

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        float sampled_temperature = CTRL_SENSOR_DEFAULT_TEMPERATURE_C;
        const bool has_new_sample = dd_temp_sensor_read_celsius(&g_temp_sensor, &sampled_temperature);

        taskENTER_CRITICAL();
        g_sensor_signals.sensor_status = dd_temp_sensor_get_status(&g_temp_sensor);

        if (has_new_sample)
        {
            ctrl_sensor_apply_new_sample(&g_sensor_signals,
                                         &g_temp_sensor,
                                         sampled_temperature);
        }
        else
        {
            ctrl_sensor_mark_invalid_sample(&g_sensor_signals);
        }
        taskEXIT_CRITICAL();

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_TASK_RECURRANCE_MS));
    }
}

void ctrl_sensor_report_task(void *params)
{
    (void)params;

    vTaskDelay(pdMS_TO_TICKS(SENSOR_REPORT_TASK_OFFSET_MS));
    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        ctrl_sensor_signals_t snapshot;
        char temperature_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char raw_temperature_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char salt_pepper_temperature_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char weighted_temperature_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char lcd_line_1[LCD_LINE_BUFFER_SIZE];
        char lcd_line_2[LCD_LINE_BUFFER_SIZE];

        taskENTER_CRITICAL();
        snapshot = g_sensor_signals;
        taskEXIT_CRITICAL();

        ctrl_sensor_format_snapshot_temperatures(&snapshot,
                             temperature_text,
                             raw_temperature_text,
                             salt_pepper_temperature_text,
                             weighted_temperature_text,
                             sizeof(temperature_text));

        ctrl_sensor_print_serial_reports(&snapshot,
                         temperature_text,
                         raw_temperature_text,
                         salt_pepper_temperature_text,
                         weighted_temperature_text);

        ctrl_sensor_build_lcd_lines(&snapshot,
                        temperature_text,
                        lcd_line_1,
                        lcd_line_2,
                        sizeof(lcd_line_1));

        ctrl_stdio_lcd_print_two_lines(lcd_line_1, lcd_line_2);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_REPORT_TASK_RECURRANCE_MS));
    }
}


