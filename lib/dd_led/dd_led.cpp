#include "dd_led.h"

void dd_led_init(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
}

void dd_led_on(uint8_t pin) {
    digitalWrite(pin, HIGH);
}

void dd_led_off(uint8_t pin) {
    digitalWrite(pin, LOW);
}

void dd_led_set(uint8_t pin, uint8_t state) {
    if (state == HIGH) {
        dd_led_on(pin);
    } else {
        dd_led_off(pin);
    }
}
