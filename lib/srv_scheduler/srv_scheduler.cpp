#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>

#include "ctrl_stdio.h"


void srv_scheduler_init(void)
{
    ctrl_stdio_serial_init();
    ctrl_stdio_lcd_init();
    ctrl_relay_task_init();

    xTaskCreate(ctrl_relay_command_task,
                "relay_cmd",
                RELAY_COMMAND_TASK_STACK_SIZE,
                NULL,
                RELAY_COMMAND_TASK_PRIORITY,
                NULL);

}


void srv_scheduler_run(void)
{

}
