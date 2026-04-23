#ifndef APP_FSM_H
#define APP_FSM_H

#include <Arduino.h>

// Inițializează automatul finit
void app_fsm_init(uint8_t led_pin);

// Evaluează starea și face tranziția dacă butonul a fost apăsat
void app_fsm_update(bool button_event, uint8_t led_pin);

#endif // APP_FSM_H