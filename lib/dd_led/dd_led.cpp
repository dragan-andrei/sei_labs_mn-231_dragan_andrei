#include "dd_led.h"

void dd_led_init(dd_led_t* led, uint8_t pin_number, void (*set_state_fn)(uint8_t, uint8_t))
{
    led->pin_number = pin_number;
    led->state = dd_led_off;
    led->set_state = set_state_fn;
    led->set_state(led->pin_number, LOW);
}

void dd_led_set_on(dd_led_t* led)
{
    led->set_state(led->pin_number, HIGH);
    led->state = dd_led_on;
}

void dd_led_set_off(dd_led_t* led)
{
    led->set_state(led->pin_number, LOW);
    led->state = dd_led_off;
}

void dd_led_toggle(dd_led_t* led)
{
    if (led->state == dd_led_on) {
        dd_led_set_off(led);
    } else {
        dd_led_set_on(led);
    }
}

