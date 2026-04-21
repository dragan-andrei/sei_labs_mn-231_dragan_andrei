#include "ctrl_task_hysteresis.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include "configs.h"
#include "ctrl_stdio.h"
#include "ctrl_hysteresis.h"
#include "ctrl_task_sensor.h"
#include "dd_button.h"
#include "dd_relay.h"
#include "dd_led.h"
#include "dd_buzzer.h"

static ctrl_hysteresis_t g_hysteresis_ctrl;
static dd_relay_t g_relay;
static dd_led_t g_warning_led;
static dd_buzzer_t g_buzzer;
static dd_button_t g_button_up;
static dd_button_t g_button_down;

static uint8_t g_hysteresis_module_initialized = CTRL_SENSOR_FLAG_DISABLED;

static uint8_t digital_read_wrapper(uint8_t pin)
{
    return (uint8_t)digitalRead(pin);
}

static void ctrl_hysteresis_format_temperature(char *buffer, size_t buffer_size, float temperature_c)
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

void ctrl_hysteresis_task_init(void)
{
    if (g_hysteresis_module_initialized != CTRL_SENSOR_FLAG_DISABLED)
    {
        return;
    }

    ctrl_hysteresis_init(&g_hysteresis_ctrl,
                         HYSTERESIS_DEFAULT_SETPOINT_C,
                         HYSTERESIS_BAND_C,
                         HYSTERESIS_SETPOINT_MIN_C,
                         HYSTERESIS_SETPOINT_MAX_C,
                         HYSTERESIS_SETPOINT_STEP_C);

    dd_relay_init(&g_relay,
                  RELAY_PIN,
                  DD_LED_OUTPUT,
                  digitalWrite,
                  pinMode);

    dd_led_init(&g_warning_led,
                LED_RED_PIN,
                DD_LED_OUTPUT,
                digitalWrite,
                pinMode);

    dd_buzzer_init(&g_buzzer, BUZZER_PIN);

    dd_button_init(&g_button_up,
                   BUTTON_UP_PIN,
                   INPUT_PULLUP,
                   digital_read_wrapper,
                   pinMode);

    dd_button_init(&g_button_down,
                   BUTTON_DOWN_PIN,
                   INPUT_PULLUP,
                   digital_read_wrapper,
                   pinMode);

    g_hysteresis_module_initialized = CTRL_SENSOR_FLAG_ENABLED;
}

/* ── Setpoint adjustment task (reads UP/DOWN buttons) ── */

void ctrl_setpoint_task(void *params)
{
    (void)params;

    ctrl_hysteresis_task_init();

    vTaskDelay(pdMS_TO_TICKS(SETPOINT_TASK_OFFSET_MS));

    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t next_check_time_up = 0;
    uint32_t next_check_time_down = 0;

    for (;;)
    {
        if (dd_button_is_pressed(&g_button_up))
        {
            if (millis() >= next_check_time_up)
            {
                next_check_time_up = millis() + BUTTON_DEBOUNCE_DELAY;

                taskENTER_CRITICAL();
                ctrl_hysteresis_increment_setpoint(&g_hysteresis_ctrl);
                taskEXIT_CRITICAL();
            }
        }

        if (dd_button_is_pressed(&g_button_down))
        {
            if (millis() >= next_check_time_down)
            {
                next_check_time_down = millis() + BUTTON_DEBOUNCE_DELAY;

                taskENTER_CRITICAL();
                ctrl_hysteresis_decrement_setpoint(&g_hysteresis_ctrl);
                taskEXIT_CRITICAL();
            }
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(SETPOINT_TASK_RECURRANCE_MS));
    }
}

/* ── Main hysteresis control loop ── */

static void ctrl_hysteresis_update_actuators(ctrl_hysteresis_state_t state)
{
    if (state == CTRL_HYSTERESIS_STATE_ON)
    {
        dd_relay_on(&g_relay);
    }
    else
    {
        dd_relay_off(&g_relay);
    }
}

static void ctrl_hysteresis_update_warning(float current_temperature_c,
                                           float set_point_c,
                                           uint32_t now_ms,
                                           uint32_t *last_buzzer_time_ms)
{
    const float deviation = current_temperature_c - set_point_c;
    const float abs_deviation = (deviation < 0.0f) ? -deviation : deviation;

    if (abs_deviation >= ALERT_DEVIATION_THRESHOLD_C)
    {
        dd_led_set_on(&g_warning_led);

        if ((now_ms - *last_buzzer_time_ms) >= BUZZER_ALERT_COOLDOWN_MS)
        {
            dd_buzzer_beep(&g_buzzer,
                           BUZZER_ALERT_FREQUENCY_HZ,
                           BUZZER_ALERT_DURATION_MS);
            *last_buzzer_time_ms = now_ms;
        }
    }
    else
    {
        dd_led_set_off(&g_warning_led);
        dd_buzzer_off(&g_buzzer);
    }
}

static void ctrl_hysteresis_print_serial_report(float current_temperature_c,
                                                float set_point_c,
                                                float low_threshold_c,
                                                float high_threshold_c,
                                                ctrl_hysteresis_state_t relay_state,
                                                uint8_t alert_active,
                                                const char *temp_text,
                                                const char *sp_text)
{
    printf("[CTRL] T=%sC | SP=%sC | Relay=%s | Alert=%c\n",
           temp_text,
           sp_text,
           (relay_state == CTRL_HYSTERESIS_STATE_ON) ? "ON" : "OFF",
           alert_active ? 'Y' : 'N');

    /* Serial Plotter format (comma-separated for Arduino Serial Plotter) */
    char low_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
    char high_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
    ctrl_hysteresis_format_temperature(low_text, sizeof(low_text), low_threshold_c);
    ctrl_hysteresis_format_temperature(high_text, sizeof(high_text), high_threshold_c);

    printf("Temperature:%s,SetPoint:%s,LowThreshold:%s,HighThreshold:%s,Relay:%u\n",
           temp_text,
           sp_text,
           low_text,
           high_text,
           (uint8_t)relay_state);
}

static void ctrl_hysteresis_update_lcd(float current_temperature_c,
                                       float set_point_c,
                                       ctrl_hysteresis_state_t relay_state,
                                       const char *temp_text,
                                       const char *sp_text)
{
    char lcd_line_1[LCD_LINE_BUFFER_SIZE];
    char lcd_line_2[LCD_LINE_BUFFER_SIZE];

    char hyst_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
    ctrl_hysteresis_format_temperature(hyst_text, sizeof(hyst_text), HYSTERESIS_BAND_C);

    snprintf(lcd_line_1,
             sizeof(lcd_line_1),
             "T:%s SP:%s",
             temp_text,
             sp_text);

    snprintf(lcd_line_2,
             sizeof(lcd_line_2),
             "Relay:%s H:%s",
             (relay_state == CTRL_HYSTERESIS_STATE_ON) ? "ON " : "OFF",
             hyst_text);

    ctrl_stdio_lcd_print_two_lines(lcd_line_1, lcd_line_2);
}

void ctrl_hysteresis_control_task(void *params)
{
    (void)params;

    ctrl_hysteresis_task_init();

    vTaskDelay(pdMS_TO_TICKS(HYSTERESIS_TASK_OFFSET_MS));

    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t last_buzzer_time_ms = 0;

    for (;;)
    {
        /* Get current temperature from sensor snapshot */
        ctrl_sensor_signals_t snapshot;
        ctrl_sensor_get_snapshot(&snapshot);

        const float current_temperature = snapshot.last_temperature_c;
        const uint8_t data_valid = snapshot.is_data_valid;

        /* Update hysteresis controller */
        ctrl_hysteresis_state_t relay_state = CTRL_HYSTERESIS_STATE_OFF;

        if (data_valid == CTRL_SENSOR_FLAG_ENABLED)
        {
            taskENTER_CRITICAL();
            relay_state = ctrl_hysteresis_update(&g_hysteresis_ctrl,
                                                 current_temperature);
            taskEXIT_CRITICAL();
        }
        else
        {
            /* Safety: turn off relay if sensor data is invalid */
            taskENTER_CRITICAL();
            g_hysteresis_ctrl.output_state = CTRL_HYSTERESIS_STATE_OFF;
            relay_state = CTRL_HYSTERESIS_STATE_OFF;
            taskEXIT_CRITICAL();
        }

        /* Apply to actuators */
        ctrl_hysteresis_update_actuators(relay_state);

        /* Read current setpoint (under critical section) */
        float set_point_c;
        float low_threshold_c;
        float high_threshold_c;

        taskENTER_CRITICAL();
        set_point_c = ctrl_hysteresis_get_setpoint(&g_hysteresis_ctrl);
        low_threshold_c = ctrl_hysteresis_get_low_threshold(&g_hysteresis_ctrl);
        high_threshold_c = ctrl_hysteresis_get_high_threshold(&g_hysteresis_ctrl);
        taskEXIT_CRITICAL();

        /* Warning LED + Buzzer for large deviation */
        const float deviation = current_temperature - set_point_c;
        const float abs_deviation = (deviation < 0.0f) ? -deviation : deviation;
        const uint8_t alert_active = (abs_deviation >= ALERT_DEVIATION_THRESHOLD_C) ? 1U : 0U;

        ctrl_hysteresis_update_warning(current_temperature,
                                       set_point_c,
                                       millis(),
                                       &last_buzzer_time_ms);

        /* Format and output */
        char temp_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        char sp_text[SENSOR_TEMPERATURE_TEXT_BUFFER_SIZE];
        ctrl_hysteresis_format_temperature(temp_text, sizeof(temp_text), current_temperature);
        ctrl_hysteresis_format_temperature(sp_text, sizeof(sp_text), set_point_c);

        ctrl_hysteresis_print_serial_report(current_temperature,
                                            set_point_c,
                                            low_threshold_c,
                                            high_threshold_c,
                                            relay_state,
                                            alert_active,
                                            temp_text,
                                            sp_text);

        ctrl_hysteresis_update_lcd(current_temperature,
                                   set_point_c,
                                   relay_state,
                                   temp_text,
                                   sp_text);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(HYSTERESIS_TASK_RECURRANCE_MS));
    }
}
