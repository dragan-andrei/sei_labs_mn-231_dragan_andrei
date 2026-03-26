#include "srv_scheduler.h"

void srv_scheduler_freertos_init(void)
{
    BaseType_t task_create_status;

    ctrl_stdio_serial_init();

    if (!ctrl_tasks_freertos_primitives_init())
    {
        printf("FreeRTOS init error: failed to create semaphore/queue\n");
        return;
    }

    /* Task 1: Button LED — checks button every 10ms, lights LED for 1s */
    task_create_status = xTaskCreate(ctrl_button_led_task_freertos,
                                     "ButtonLED",
                                     FREERTOS_FIRST_TASK_STACK_SIZE,
                                     NULL,
                                     FREERTOS_FIRST_TASK_PRIORITY,
                                     NULL
                                    );
    if (task_create_status != pdPASS)
    {
        printf("FreeRTOS init error: failed to create Task 1\n");
        return;
    }

    /* Task 2: Sync — waits for semaphore, sends N bytes, blinks LED N times */
    task_create_status = xTaskCreate(ctrl_sync_task_freertos,
                                     "SyncTask",
                                     FREERTOS_SECOND_TASK_STACK_SIZE,
                                     NULL,
                                     FREERTOS_SECOND_TASK_PRIORITY,
                                     NULL
                                    );
    if (task_create_status != pdPASS)
    {
        printf("FreeRTOS init error: failed to create Task 2\n");
        return;
    }

    /* Task 3: Async — reads queue every 200ms, prints to Serial */
    task_create_status = xTaskCreate(ctrl_async_task_freertos,
                                     "AsyncTask",
                                     FREERTOS_THIRD_TASK_STACK_SIZE,
                                     NULL,
                                     FREERTOS_THIRD_TASK_PRIORITY,
                                     NULL
                                    );
    if (task_create_status != pdPASS)
    {
        printf("FreeRTOS init error: failed to create Task 3\n");
        return;
    }

    vTaskStartScheduler();
}
    