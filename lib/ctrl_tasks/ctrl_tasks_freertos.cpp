#include "ctrl_tasks.h"

SemaphoreHandle_t xButtonLedSemaphore;
QueueHandle_t xSendDataQueue;

void ctrl_button_led_task_init(dd_button_t *button, dd_led_t *led)
{
    dd_led_init(led,
                    LED_GREEN_PIN,
                    DD_LED_OUTPUT,
                    digitalWrite,
                    pinMode
                    );

    dd_button_init(button,
                    BUTTON_PIN,
                    INPUT_PULLUP,
                    digitalRead,
                    pinMode
                    );
    xButtonLedSemaphore = xSemaphoreCreateBinary();                
}

void ctrl_button_led_task(void *params)
{
    static uint8_t need_init = true;

    static dd_led_t led;
    static dd_button_t button;

    static TickType_t next_button_check_time = 0;
    static TickType_t led_off_time = 0;
    static uint8_t led_on_flag = false;

    if(need_init)
    {
        ctrl_button_led_task_init(&button, &led);
        need_init = false;
    }

    while (true)
    {
        if(xTaskGetTickCount() >= next_button_check_time)
        {
            if (dd_button_is_pressed(&button))
            {
                xSemaphoreGive(xButtonLedSemaphore);
                dd_led_set_on(&led);
                led_off_time = xTaskGetTickCount() + pdMS_TO_TICKS(FREERTOS_FIRST_ON_TIME_MS);
                led_on_flag = true;
            }
            next_button_check_time = xTaskGetTickCount() + pdMS_TO_TICKS(BUTTON_DEBOUNCE_DELAY);
        }
        
        if(led_on_flag && xTaskGetTickCount() >= led_off_time)
        {
            dd_led_set_off(&led);
            led_on_flag = false;
        }

        vTaskDelay(pdMS_TO_TICKS(FREERTOS_FIRST_RECURRANCE_MS));
    }
    
}
    
    