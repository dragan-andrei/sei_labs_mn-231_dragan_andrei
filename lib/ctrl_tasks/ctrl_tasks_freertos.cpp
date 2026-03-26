#include "ctrl_tasks.h"

SemaphoreHandle_t xButtonLedSemaphore;
QueueHandle_t xSendDataQueue;

/* ======== Task 1: Button LED (period 10ms) ======== */

bool ctrl_tasks_freertos_primitives_init(void)
{
    xButtonLedSemaphore = xSemaphoreCreateBinary();
    if (xButtonLedSemaphore == NULL)
    {
        return false;
    }

    xSendDataQueue = xQueueCreate(FREERTOS_QUEUE_SIZE, sizeof(uint8_t));
    if (xSendDataQueue == NULL)
    {
        return false;
    }

    return true;
}

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
}

void ctrl_button_led_task_freertos(void *params)
{
    dd_led_t led;
    dd_button_t button;
    TickType_t led_off_time = 0;
    bool led_on_flag = false;
    bool press_latched = false;
    bool release_timing_active = false;
    TickType_t release_stable_start_time = 0;
    TickType_t last_press_event_time = 0;
    const TickType_t debounce_delay_ticks = pdMS_TO_TICKS(FREERTOS_BUTTON_DEBOUNCE_DELAY_MS);

    ctrl_button_led_task_freertos_init(&button, &led);

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (true)
    {
        TickType_t current_time = xTaskGetTickCount();
        bool raw_button_pressed = dd_button_is_pressed(&button);

        if (raw_button_pressed)
        {
            release_timing_active = false;

            if (!press_latched && ((current_time - last_press_event_time) >= debounce_delay_ticks))
            {
                xSemaphoreGive(xButtonLedSemaphore);
                dd_led_set_on(&led);
                led_off_time = current_time + pdMS_TO_TICKS(FREERTOS_FIRST_ON_TIME_MS);
                led_on_flag = true;
                press_latched = true;
                last_press_event_time = current_time;
            }
        }
        else
        {
            if (!release_timing_active)
            {
                release_timing_active = true;
                release_stable_start_time = current_time;
            }
            else if (press_latched && ((current_time - release_stable_start_time) >= debounce_delay_ticks))
            {
                press_latched = false;
            }
        }

        if (led_on_flag && current_time >= led_off_time)
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
    dd_led_init(&led, LED_BLUE_PIN, DD_LED_OUTPUT, digitalWrite, pinMode);

    uint8_t N = FREERTOS_SECOND_COUNTER_START_VALUE;

    while (true)
    {
        /* Block until Task 1 gives the semaphore (button press) */
        xSemaphoreTake(xButtonLedSemaphore, portMAX_DELAY);

        N++;

        /* Send N bytes (1,2,3,...,N) into the queue with 50ms interval */
        for (uint8_t i = FREERTOS_SECOND_SEQUENCE_FIRST_VALUE; i <= N; i++)
        {
            xQueueSendToBack(xSendDataQueue, &i, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(FREERTOS_SECOND_SEND_INTERVAL_MS));
        }

        /* Send terminator byte 0 */
        uint8_t zero = FREERTOS_QUEUE_TERMINATOR_VALUE;
        xQueueSendToBack(xSendDataQueue, &zero, portMAX_DELAY);

        /* Blink LED N times: ON 300ms, OFF 500ms */
        for (uint8_t i = FREERTOS_SECOND_COUNTER_START_VALUE; i < N; i++)
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
    printf("=== FreeRTOS Lab Started ===\n");

    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(FREERTOS_THIRD_RECURRANCE_MS));

        uint8_t byte_val;
        while (xQueueReceive(xSendDataQueue, &byte_val, FREERTOS_QUEUE_READ_TIMEOUT_TICKS) == pdTRUE)
        {
            if (byte_val == FREERTOS_QUEUE_TERMINATOR_VALUE)
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
    
    