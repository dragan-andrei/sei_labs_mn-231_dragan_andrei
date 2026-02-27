#include "srv_scheduler.h"

void srv_scheduler_freertos_init(void)
{
    xTaskCreate(ctrl_button_led_task_freertos, 
                "ButtonLED", 
                FREERTOS_FIRST_TASK_STACK_SIZE, 
                NULL, 
                FREERTOS_FIRST_TASK_PRIORITY, 
                NULL
            );

    vTaskStartScheduler();
}
    