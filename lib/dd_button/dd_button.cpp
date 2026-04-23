#include "dd_button.h"
#include <Arduino.h>

void dd_button_init(dd_button_t* btn, uint8_t pin, uint32_t debounce_ms) {
    if (!btn) return;
    
    btn->pin = pin;
    btn->last_debounce_time = 0;
    btn->last_read_state = false;
    btn->stable_state = false;
    btn->debounce_delay_ms = debounce_ms;
    
    // Folosim INPUT_PULLUP
    pinMode(pin, INPUT_PULLUP);
}

bool dd_button_is_pressed_raw(uint8_t pin) {
    // Datorită PULLUP, butonul neapăsat citește HIGH (1). 
    // Când e apăsat și face contact cu GND, citește LOW (0).
    return (digitalRead(pin) == LOW);
}

bool dd_button_update(dd_button_t* btn, uint32_t current_time_ms) {
    if (!btn) return false;
    
    bool current_btn_read = dd_button_is_pressed_raw(btn->pin);
    bool trigger_event = false;

    // Reset timer dacă s-a schimbat starea (zgomot)
    if (current_btn_read != btn->last_read_state) {
        btn->last_debounce_time = current_time_ms;
    }

    // Validăm dacă timpul scurs e peste mărimea filtrului (debounce)
    if ((current_time_ms - btn->last_debounce_time) > btn->debounce_delay_ms) {
        if (current_btn_read != btn->stable_state) {
            btn->stable_state = current_btn_read;
            
            // Generăm eveniment DOAR la apăsare (tranziție false -> true)
            if (btn->stable_state == true) {
                trigger_event = true;
            }
        }
    }
    
    btn->last_read_state = current_btn_read;
    return trigger_event;
}