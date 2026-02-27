#include "ctrl_tasks.h"

static volatile uint8_t green_led_state = DD_LED_OFF;
static volatile uint16_t red_led_frequency = DEFAULT_BLINK_FREQUENCY;
static volatile uint8_t red_led_state = DD_LED_OFF;

static uint8_t digital_read_wrapper(uint8_t pin)
{
    return (uint8_t)digitalRead(pin);
}

void ctrl_button_led_task_init(dd_button_t *button, dd_led_t *led)
{
    dd_button_init(button,
                    BUTTON_PIN,
                    INPUT_PULLUP,
                    digital_read_wrapper,
                    pinMode
                    );
    dd_led_init(led,
                    LED_GREEN_PIN,
                    DD_LED_OUTPUT,
                    digitalWrite,
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
                return;
            }

            next_check_time = millis() + BUTTON_DEBOUNCE_DELAY;
            green_led_state = !green_led_state;
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
                    DD_LED_OUTPUT,
                    digitalWrite,
                    pinMode
                    );
}

void ctrl_blink_led_task(void *params)
{
    static dd_led_t led;
    static uint8_t need_init = true;
    static uint32_t next_toggle_time = 0;

    if(need_init)
    {
        ctrl_blink_led_task_init(&led);
        need_init = false;
    }

    if (green_led_state == DD_LED_OFF)
    {
        if (millis() >= next_toggle_time)
        {
            dd_led_toggle(&led);
            red_led_state = led.state;
            next_toggle_time = millis() + red_led_frequency;
        }
    }
    else
    {
        dd_led_set_off(&led);
        red_led_state = DD_LED_OFF;
        next_toggle_time = 0;
    }
}

void ctrl_inc_dec_led_task_init(dd_button_t *button_up, dd_button_t *button_down)
{
    dd_button_init(button_up,
                    BUTTON_UP_PIN,
                    INPUT_PULLUP,
                    digital_read_wrapper,
                    pinMode
                    );
    dd_button_init(button_down,
                    BUTTON_DOWN_PIN,
                    INPUT_PULLUP,
                    digital_read_wrapper,
                    pinMode
                    );
}

void ctrl_inc_dec_led_task(void *params)
{
    static dd_button_t button_up;
    static dd_button_t button_down;
    static uint8_t need_init = true;
    static uint32_t next_check_time_up = 0;
    static uint32_t next_check_time_down = 0;

    if(need_init)
    {
        ctrl_inc_dec_led_task_init(&button_up, &button_down);
        need_init = false;
    }

    if(dd_button_is_pressed(&button_up))
    {
        if (millis() >= next_check_time_up)
        {
            next_check_time_up = millis() + BUTTON_DEBOUNCE_DELAY;

            if (red_led_frequency > MIN_BLINK_FREQUENCY)
            {
                red_led_frequency -= BLINK_FREQUENCY_STEP;
            }
        }
    }

    if(dd_button_is_pressed(&button_down))
    {
        if (millis() >= next_check_time_down)
        {
            next_check_time_down = millis() + BUTTON_DEBOUNCE_DELAY;

            if (red_led_frequency < MAX_BLINK_FREQUENCY)
            {
                red_led_frequency += BLINK_FREQUENCY_STEP;
            }
        }
    }
}

void ctrl_idle_task_init(void)
{
    ctrl_stdio_serial_init();
}

void ctrl_idle_task(void *params)
{
    static uint8_t need_init = true;
    static uint32_t next_print_time = 0;

    if(need_init)
    {
        ctrl_idle_task_init();
        need_init = false;
    }

    if (millis() >= next_print_time)
    {
        printf("Green LED: %s | Red LED: %s | Blink freq: %u ms\n",
               green_led_state ? "ON" : "OFF",
               red_led_state ? "ON" : "OFF",
               red_led_frequency);
        next_print_time = millis() + 500;
    }
}
