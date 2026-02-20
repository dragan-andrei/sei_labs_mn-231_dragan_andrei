#include "ctrl_tasks.h"

 static dd_button_t button;
    static dd_led_t led;

void ctrl_button_led_task_init(  dd_button_t *button,  dd_led_t *led)
{
    // Initialization code for the button and LED can be added here if needed
    dd_button_init(button, KEYPAD_PIN_1, DD_BUTTON_INPUT_PULLUP, [](uint8_t pin) -> uint8_t { return (uint8_t)digitalRead(pin); }, pinMode);
    dd_led_init(led, LED_RED_PIN, digitalWrite);

}

void ctrl_button_led_task(void *params)
{

    static uint8_t need_init = true;

    if(need_init) {
        ctrl_button_led_task_init(&button, &led);
        need_init = false;
    }
    if (dd_button_is_pressed(&button)) {
        dd_led_set_on(&led);
    } else {
        dd_led_set_off(&led);
    }
}