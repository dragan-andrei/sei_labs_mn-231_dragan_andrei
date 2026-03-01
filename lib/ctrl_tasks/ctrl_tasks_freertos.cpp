#include "ctrl_tasks.h"

SemaphoreHandle_t xButtonLedSemaphore;
QueueHandle_t xSendDataQueue;

/* ======== Task 1: Button LED (period 10ms) ======== */

static void ctrl_button_led_task_freertos_init(dd_button_t *button, dd_led_t *led)
{
    dd_button_init(button,
                    BUTTON_PIN,
                    INPUT_PULLUP,
                    digitalRead,
                    pinMode
                    );
    dd_led_init(led,
                    LED_GREEN_PIN,
                    DD_LED_OUTPUT,
                    digitalWrite,
                    pinMode
                    );
    xButtonLedSemaphore = xSemaphoreCreateBinary();
    xSendDataQueue = xQueueCreate(FREERTOS_QUEUE_SIZE, sizeof(uint8_t));
}

void ctrl_button_led_task_freertos(void *params)
{
    dd_led_t led;
    dd_button_t button;
    TickType_t led_off_time = 0;
    uint8_t led_on_flag = false;

    ctrl_button_led_task_freertos_init(&button, &led);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        if (dd_button_is_pressed(&button))
        {
            xSemaphoreGive(xButtonLedSemaphore);
            dd_led_set_on(&led);
            led_off_time = xTaskGetTickCount() + pdMS_TO_TICKS(FREERTOS_FIRST_ON_TIME_MS);
            led_on_flag = true;
        }

        if (led_on_flag && xTaskGetTickCount() >= led_off_time)
        {
            dd_led_set_off(&led);
            led_on_flag = false;
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(FREERTOS_FIRST_RECURRANCE_MS));
    }
}

/* ======== Task 2: Sync – waits for semaphore, sends N bytes, blinks LED N times ======== */

void ctrl_sync_task_freertos(void *params)
{
    dd_led_t led;
    dd_led_init(&led, LED_RED_PIN, DD_LED_OUTPUT, digitalWrite, pinMode);

    uint8_t N = 0;

    while (true)
    {
        /* Block until Task 1 gives the semaphore (button press) */
        xSemaphoreTake(xButtonLedSemaphore, portMAX_DELAY);

        N++;

        /* Send N bytes (1,2,3,...,N) into the queue with 50ms interval */
        for (uint8_t i = 1; i <= N; i++)
        {
            xQueueSendToBack(xSendDataQueue, &i, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(FREERTOS_SECOND_SEND_INTERVAL_MS));
        }

        /* Send terminator byte 0 */
        uint8_t zero = 0;
        xQueueSendToBack(xSendDataQueue, &zero, portMAX_DELAY);

        /* Blink LED N times: ON 300ms, OFF 500ms */
        for (uint8_t i = 0; i < N; i++)
        {
            dd_led_set_on(&led);
            vTaskDelay(pdMS_TO_TICKS(FREERTOS_SECOND_LED_ON_MS));
            dd_led_set_off(&led);
            vTaskDelay(pdMS_TO_TICKS(FREERTOS_SECOND_LED_OFF_MS));
        }
    }
}

/* ======== Task 3: Async – reads queue every 200ms, prints to Serial ======== */

void ctrl_async_task_freertos(void *params)
{
    ctrl_stdio_serial_init();
    printf("=== FreeRTOS Lab Started ===\n");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(FREERTOS_THIRD_RECURRANCE_MS));

        uint8_t byte_val;
        while (xQueueReceive(xSendDataQueue, &byte_val, 0) == pdTRUE)
        {
            if (byte_val == 0)
            {
                printf("\n");
            }
            else
            {
                printf("%d ", byte_val);
            }
        }
    }
}
    
    