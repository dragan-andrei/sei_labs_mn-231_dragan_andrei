#ifndef DD_LED_H
#define DD_LED_H

#include <Arduino.h>

// Inițializează pinul ca ieșire și stinge LED-ul
void dd_led_init(uint8_t pin);

// Aprinde LED-ul
void dd_led_on(uint8_t pin);

// Stinge LED-ul
void dd_led_off(uint8_t pin);

// Setează direct starea LED-ului (HIGH/LOW) pentru compatibilitate
void dd_led_set(uint8_t pin, uint8_t state);

#endif // DD_LED_H