#include "ctrl_task_motor.h"

#include <Arduino_FreeRTOS.h>

#include "configs.h"
#include "ctrl_motor.h"
#include "ctrl_stdio.h"

void ctrl_motor_task_init(void)
{
    ctrl_motor_init();
}

void ctrl_motor_command_task(void *params)
{
    TickType_t last_wake_time = xTaskGetTickCount();
#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
    uint32_t last_diagnostics_log_time_ms = 0U;
#endif

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_COMMAND_TASK_OFFSET_MS));
    printf("[TASK][motor_cmd] started\n");

    for (;;)
    {
        char serial_character = '\0';
        char keypad_character = '\0';
        uint8_t serial_processed = 0U;
        uint8_t keypad_processed = 0U;

        while (serial_processed < MOTOR_INPUT_PROCESS_BUDGET &&
               ctrl_stdio_serial_read_char(&serial_character) != false)
        {
            ctrl_motor_handle_serial_char(serial_character);
            serial_processed++;
        }

        while (keypad_processed < MOTOR_INPUT_PROCESS_BUDGET &&
               ctrl_stdio_keypad_read_char(&keypad_character) != false)
        {
            ctrl_motor_handle_keypad_char(keypad_character);
            keypad_processed++;
        }

        ctrl_motor_flush_serial_if_idle();

#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
        if ((uint32_t)(millis() - last_diagnostics_log_time_ms) >= MOTOR_RUNTIME_DIAGNOSTICS_PERIOD_MS)
        {
            last_diagnostics_log_time_ms = millis();
            printf("[TASK][motor_cmd] alive hw=%u\n", (unsigned int)uxTaskGetStackHighWaterMark(NULL));
        }
#endif

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_COMMAND_TASK_RECURRANCE_MS));
    }
}

void ctrl_motor_control_task(void *params)
{
    TickType_t last_wake_time = xTaskGetTickCount();
#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
    uint32_t last_diagnostics_log_time_ms = 0U;
#endif

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_CONTROL_TASK_OFFSET_MS));

    for (;;)
    {
        ctrl_motor_control_step();

#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
        if ((uint32_t)(millis() - last_diagnostics_log_time_ms) >= MOTOR_RUNTIME_DIAGNOSTICS_PERIOD_MS)
        {
            last_diagnostics_log_time_ms = millis();
            printf("[TASK][motor_ctrl] alive hw=%u\n", (unsigned int)uxTaskGetStackHighWaterMark(NULL));
        }
#endif

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_CONTROL_TASK_RECURRANCE_MS));
    }
}

void ctrl_motor_status_task(void *params)
{
    TickType_t last_wake_time = xTaskGetTickCount();
#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
    uint32_t last_diagnostics_log_time_ms = 0U;
#endif

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_STATUS_TASK_OFFSET_MS));

    for (;;)
    {
        ctrl_motor_report_status_if_due();

#if (MOTOR_RUNTIME_DIAGNOSTICS_ENABLED == 1)
        if ((uint32_t)(millis() - last_diagnostics_log_time_ms) >= MOTOR_RUNTIME_DIAGNOSTICS_PERIOD_MS)
        {
            last_diagnostics_log_time_ms = millis();
            printf("[TASK][motor_stat] alive hw=%u\n", (unsigned int)uxTaskGetStackHighWaterMark(NULL));
        }
#endif

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_STATUS_TASK_RECURRANCE_MS));
    }
}
