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
    uint8_t is_data_valid;
    uint8_t is_alert_active;
    uint32_t sample_count;
    dd_temp_sensor_status_t sensor_status;
} ctrl_sensor_signals_t;

static dd_temp_sensor_t g_temp_sensor;
static ctrl_sensor_signals_t g_sensor_signals;
static uint8_t g_sensor_module_initialized = 0;

static const char *ctrl_sensor_status_to_text(dd_temp_sensor_status_t status)
{
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

static const char *ctrl_sensor_status_to_short_text(dd_temp_sensor_status_t status)
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

void ctrl_sensor_task_init(void)
{
    if (g_sensor_module_initialized != 0U)
    {
        return;
    }

    dd_temp_sensor_init(&g_temp_sensor, TEMP_SENSOR_PIN);

    g_sensor_signals.last_temperature_c = 0.0f;
    g_sensor_signals.is_data_valid = 0;
    g_sensor_signals.is_alert_active = 0;
    g_sensor_signals.sample_count = 0;
    g_sensor_signals.sensor_status = dd_temp_sensor_get_status(&g_temp_sensor);

    g_sensor_module_initialized = 1;
}

void ctrl_sensor_acquisition_task(void *params)
{
    (void)params;

    ctrl_sensor_task_init();

    vTaskDelay(pdMS_TO_TICKS(SENSOR_TASK_OFFSET_MS));

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        float sampled_temperature = 0.0f;
        const bool has_new_sample = dd_temp_sensor_read_celsius(&g_temp_sensor, &sampled_temperature);

        taskENTER_CRITICAL();
        g_sensor_signals.sensor_status = dd_temp_sensor_get_status(&g_temp_sensor);

        if (has_new_sample)
        {
            g_sensor_signals.last_temperature_c = sampled_temperature;
            g_sensor_signals.is_data_valid = 1;
            g_sensor_signals.sample_count++;

            if (g_sensor_signals.is_alert_active == 0U)
            {
                if (sampled_temperature >= SENSOR_ALERT_THRESHOLD_C)
                {
                    g_sensor_signals.is_alert_active = 1;
                }
            }
            else
            {
                if (sampled_temperature <= (SENSOR_ALERT_THRESHOLD_C - SENSOR_ALERT_HYSTERESIS_C))
                {
                    g_sensor_signals.is_alert_active = 0;
                }
            }
        }
        else
        {
            g_sensor_signals.is_data_valid = 0;
            g_sensor_signals.is_alert_active = 0;
        }
        taskEXIT_CRITICAL();

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SENSOR_TASK_RECURRANCE_MS));
    }
}

void ctrl_sensor_report_task(void *params)
{
    (void)params;

    vTaskDelay(pdMS_TO_TICKS(SENSOR_REPORT_TASK_OFFSET_MS));

    for (;;)
    {
        ctrl_sensor_signals_t snapshot;
        char temperature_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char lcd_line_1[LCD_LINE_BUFFER_SIZE];
        char lcd_line_2[LCD_LINE_BUFFER_SIZE];

        taskENTER_CRITICAL();
        snapshot = g_sensor_signals;
        taskEXIT_CRITICAL();

        ctrl_sensor_format_temperature(temperature_text,
                                       sizeof(temperature_text),
                                       snapshot.last_temperature_c);

        printf("[SENSOR] T=%s C | status=%s | valid=%u | alert=%u | samples=%lu\n",
               temperature_text,
               ctrl_sensor_status_to_text(snapshot.sensor_status),
               snapshot.is_data_valid,
               snapshot.is_alert_active,
               (unsigned long)snapshot.sample_count);

        snprintf(lcd_line_1,
                 sizeof(lcd_line_1),
                 "T:%sC %s",
                 temperature_text,
                 ctrl_sensor_status_to_short_text(snapshot.sensor_status));

        snprintf(lcd_line_2,
                 sizeof(lcd_line_2),
                 "AL:%c N:%lu",
                 snapshot.is_alert_active ? 'Y' : 'N',
                 (unsigned long)snapshot.sample_count);

        ctrl_stdio_lcd_print_two_lines(lcd_line_1, lcd_line_2);

        vTaskDelay(pdMS_TO_TICKS(SENSOR_REPORT_TASK_RECURRANCE_MS));
    }
}


