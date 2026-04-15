#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>

#include "ctrl_stdio.h"


void srv_scheduler_init(void)
{
    BaseType_t command_task_created = pdFAIL;
    BaseType_t control_task_created = pdFAIL;
    BaseType_t status_task_created = pdPASS;

    ctrl_stdio_serial_init();

#if (LCD_STATUS_OUTPUT_ENABLED == 1)
    ctrl_stdio_lcd_init();
#else
    printf("[SCHED] LCD status output disabled\n");
#endif

    ctrl_motor_task_init();

    command_task_created = xTaskCreate(ctrl_motor_command_task,
                                       "motor_cmd",
                                       MOTOR_COMMAND_TASK_STACK_SIZE,
                                       NULL,
                                       MOTOR_COMMAND_TASK_PRIORITY,
                                       NULL);

    control_task_created = xTaskCreate(ctrl_motor_control_task,
                                       "motor_ctrl",
                                       MOTOR_CONTROL_TASK_STACK_SIZE,
                                       NULL,
                                       MOTOR_CONTROL_TASK_PRIORITY,
                                       NULL);

#if (MOTOR_STATUS_TASK_ENABLED == 1)
    status_task_created = xTaskCreate(ctrl_motor_status_task,
                                      "motor_stat",
                                      MOTOR_STATUS_TASK_STACK_SIZE,
                                      NULL,
                                      MOTOR_STATUS_TASK_PRIORITY,
                                      NULL);
#else
    printf("[SCHED] motor_stat task disabled\n");
#endif

    if (command_task_created != pdPASS)
    {
        printf("[SCHED] failed to create motor_cmd task\n");
    }

    if (control_task_created != pdPASS)
    {
        printf("[SCHED] failed to create motor_ctrl task\n");
    }

    if (status_task_created != pdPASS)
    {
        printf("[SCHED] failed to create motor_stat task\n");
    }

    if (command_task_created == pdPASS &&
        control_task_created == pdPASS &&
        status_task_created == pdPASS)
    {
        printf("[SCHED] motor tasks created\n");
    }
}


void srv_scheduler_run(void)
{

}
