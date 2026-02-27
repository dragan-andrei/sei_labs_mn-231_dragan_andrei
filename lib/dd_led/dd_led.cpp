#include "dd_led.h"

void dd_led_init(dd_led_t* led,
                 uint8_t pin_number,
                 uint8_t mode,
                 void (*set_state)(uint8_t, uint8_t),
                 void (*set_pin_mode)(uint8_t, uint8_t))
{
    if (led == NULL || set_state == NULL || set_pin_mode == NULL) {
        return;
    }

    led->pin_number = pin_number;
    led->state = DD_LED_OFF;
    led->set_state = set_state;
    led->set_pin_mode = set_pin_mode;

    led->set_pin_mode(led->pin_number, mode);
    led->set_state(led->pin_number, LOW);
}

void dd_led_set_on(dd_led_t* led)
{
    if (led == NULL || led->set_state == NULL) {
        return;
    }

    led->set_state(led->pin_number, HIGH);
    led->state = DD_LED_ON;
}

void dd_led_set_off(dd_led_t* led)
{
    if (led == NULL || led->set_state == NULL) {
        return;
    }

    led->set_state(led->pin_number, LOW);
    led->state = DD_LED_OFF;
}

void dd_led_toggle(dd_led_t* led)
{
    if (led == NULL) {
        return;
    }

    if (led->state == DD_LED_ON) {
        dd_led_set_off(led);
    } else {
        dd_led_set_on(led);
    }
}

