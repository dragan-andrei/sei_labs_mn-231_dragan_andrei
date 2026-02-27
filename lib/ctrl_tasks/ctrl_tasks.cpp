#include "ctrl_tasks.h"

 static uint8_t green_led_state = DD_LED_OFF;
 static uint8_t red_led_frequency = DEFAULT_BLINK_FREQUENCY;

void ctrl_button_led_task_init(  dd_button_t *button,  dd_led_t *led)
{
    
    dd_button_init(button, 
                    BUTTON_PIN, 
                    digitalRead, 
                    pinMode
                    );
    dd_led_init(led, 
                    LED_GREEN_PIN, 
                    digitalWrite
                    pinMode
                );

}

void ctrl_button_led_task(void *params)
{
    static dd_button_t button;
    static dd_led_t led;
    static uint8_t need_init = true;
    static uint32_t next_check_time = 0;

    if(need_init) 
    {
        ctrl_button_led_task_init(&button, &led);
        need_init = false;
    }

    if (dd_button_is_pressed(&button)) 
    {
            if(millis() < next_check_time) 
            {
                return; // Skip if we're still within the debounce period
            }

            next_check_time = millis() + BUTTON_DEBOUNCE_DELAY; // Set next check time
            green_led_state = !green_led_state; // Toggle LED state
    }

    if (green_led_state) 
    {
            dd_led_set_on(&led);
    } 
        else 
        {
            dd_led_set_off(&led);
        }
}       

void ctrl_blink_led_task_init(dd_led_t *led)
{
    dd_led_init(led, 
                    LED_RED_PIN, 
                    digitalWrite,
                    pinMode
                    );
}

void ctrl_blink_led_task(void *params)
{
    static dd_led_t led;
    static uint8_t need_init = true;
    static uint32_t last_toggle_time = 0;

    if(need_init) 
    {
        ctrl_blink_led_task_init(&led);
        need_init = false;
    }

    if (millis() >= next_toggle_time && 
        green_led_state == DD_LED_OFF
    ) 
    {
        dd_led_toggle(&led);
        last_toggle_time = millis() + red_led_frequency; // Schedule next toggle based on current frequency
    }
    {
        dd_led_toggle(&led);
        next_toggle_time = millis() + red_led_frequency; // Schedule next toggle based on current frequency
    }
}

void ctrl_inc_dec_led_task_init(dd_button_t *button_up, dd_button_t *button_down)
{
    dd_button_init(button_up, 
                    BUTTON_UP_PIN, 
                    digitalRead, 
                    pinMode
                    );
    dd_button_init(button_down, 
                    BUTTON_DOWN_PIN,
                    digitalRead,
                    pinMode
                    );
}

void ctrl_inc_dec_led_task(void *params)
{
    static dd_button_t button_up;
    static dd_button_t button_down;
    static uint8_t need_init = true;
    static uint32_t next_check_time = 0;

    if(need_init) 
    {
        ctrl_inc_dec_led_task_init(&button_up, &button_down);
        need_init = false;
    }

    if(dd_button_is_pressed(&button_up)) 
    {
        if (millis() < next_check_time) 
        {
            return; // Skip if we're still within the debounce period
        }

        next_check_time = millis() + BUTTON_DEBOUNCE_DELAY; // Set next check time

        if (red_led_frequency > MIN_BLINK_FREQUENCY) 
        {
            red_led_frequency -= BLINK_FREQUENCY_STEP; // Increase blink frequency
        }
    }

    if(dd_button_is_pressed(&button_down)) 
    {
        if (millis() < next_check_time) 
        {
            return; // Skip if we're still within the debounce period
        }

        next_check_time = millis() + BUTTON_DEBOUNCE_DELAY; // Set next check time

        if (red_led_frequency < MAX_BLINK_FREQUENCY) 
        {
            red_led_frequency += BLINK_FREQUENCY_STEP; // Decrease blink frequency
        }
    }

void ctrl_idle_task_init(void)
{
    ctrl_stdio_serial_init();
}

void ctrl_idle_task(void *params)
{      
    // Idle task can be used for background processing or simply to keep the CPU active

}
