#include "dd_button.h"

void dd_button_init(uint8_t pin) {
    // Folosim INPUT_PULLUP pentru a nu avea nevoie de rezistor extern pe breadboard
    pinMode(pin, INPUT_PULLUP);
}

bool dd_button_is_pressed(uint8_t pin) {
    // Datorită PULLUP, butonul neapăsat citește HIGH (1). 
    // Când e apăsat și face contact cu GND, citește LOW (0).
    return (digitalRead(pin) == LOW);
}