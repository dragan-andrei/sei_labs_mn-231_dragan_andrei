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

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_COMMAND_TASK_OFFSET_MS));

    for (;;)
    {
        char serial_character = '\0';
        char keypad_character = '\0';

        while (ctrl_stdio_serial_read_char(&serial_character) != false)
        {
            ctrl_motor_handle_serial_char(serial_character);
        }

        while (ctrl_stdio_keypad_read_char(&keypad_character) != false)
        {
            ctrl_motor_handle_keypad_char(keypad_character);
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_COMMAND_TASK_RECURRANCE_MS));
    }
}

void ctrl_motor_control_task(void *params)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_CONTROL_TASK_OFFSET_MS));

    for (;;)
    {
        ctrl_motor_control_step();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_CONTROL_TASK_RECURRANCE_MS));
    }
}

void ctrl_motor_status_task(void *params)
{
    TickType_t last_wake_time = xTaskGetTickCount();

    (void)params;

    vTaskDelay(pdMS_TO_TICKS(MOTOR_STATUS_TASK_OFFSET_MS));

    for (;;)
    {
        ctrl_motor_report_status_if_due();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MOTOR_STATUS_TASK_RECURRANCE_MS));
    }
}
