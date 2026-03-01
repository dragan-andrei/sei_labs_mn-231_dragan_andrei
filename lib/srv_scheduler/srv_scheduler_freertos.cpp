#include "srv_scheduler.h"

void srv_scheduler_freertos_init(void)
{
    /* Task 1: Button LED — checks button every 10ms, lights LED for 1s */
    xTaskCreate(ctrl_button_led_task_freertos,
                "ButtonLED",
                FREERTOS_FIRST_TASK_STACK_SIZE,
                NULL,
                FREERTOS_FIRST_TASK_PRIORITY,
                NULL
            );

    /* Task 2: Sync — waits for semaphore, sends N bytes, blinks LED N times */
    xTaskCreate(ctrl_sync_task_freertos,
                "SyncTask",
                FREERTOS_SECOND_TASK_STACK_SIZE,
                NULL,
                FREERTOS_SECOND_TASK_PRIORITY,
                NULL
            );

    /* Task 3: Async — reads queue every 200ms, prints to Serial */
    xTaskCreate(ctrl_async_task_freertos,
                "AsyncTask",
                FREERTOS_THIRD_TASK_STACK_SIZE,
                NULL,
                FREERTOS_THIRD_TASK_PRIORITY,
                NULL
            );

    vTaskStartScheduler();
}
    