#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>

#include "ctrl_stdio.h"


void srv_scheduler_init(void)
{
    ctrl_stdio_serial_init();
    ctrl_stdio_lcd_init();
    ctrl_motor_task_init();

    xTaskCreate(ctrl_motor_command_task,
                "motor_cmd",
                MOTOR_COMMAND_TASK_STACK_SIZE,
                NULL,
                MOTOR_COMMAND_TASK_PRIORITY,
                NULL);

    xTaskCreate(ctrl_motor_control_task,
                "motor_ctrl",
                MOTOR_CONTROL_TASK_STACK_SIZE,
                NULL,
                MOTOR_CONTROL_TASK_PRIORITY,
                NULL);

    xTaskCreate(ctrl_motor_status_task,
                "motor_stat",
                MOTOR_STATUS_TASK_STACK_SIZE,
                NULL,
                MOTOR_STATUS_TASK_PRIORITY,
                NULL);
}


void srv_scheduler_run(void)
{

}
