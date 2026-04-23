#include "dd_led.h"

void dd_led_init(uint8_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW); // Ne asigurăm că la pornire LED-ul este stins
}

void dd_led_on(uint8_t pin) {
    digitalWrite(pin, HIGH);
}

void dd_led_off(uint8_t pin) {
    digitalWrite(pin, LOW);
} 

// Acest fișier implementează funcțiile pentru controlul LED-urilor, folosind funcțiile standard din Arduino.h pentru a seta modul pinului și a controla starea acestuia.