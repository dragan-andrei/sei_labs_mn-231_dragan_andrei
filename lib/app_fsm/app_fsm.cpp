#include "app_fsm.h"
#include "dd_led.h"
#include "ctrl_stdio.h"
#include <Arduino.h> // Pentru Serial.println

typedef enum {
    STATE_OFF = 0,
    STATE_ON
} FSM_State_t;

static FSM_State_t current_state = STATE_OFF;

void app_fsm_init(uint8_t led_pin) {
    dd_led_init(led_pin);
    current_state = STATE_OFF;
    Serial.printf("[FSM INIT] State = 0 (OFF), LED Pin = %d\n", led_pin); 
}

void app_fsm_update(bool button_event, uint8_t led_pin) {
    if (!button_event) return; 

    switch (current_state) {
        case STATE_OFF:
            current_state = STATE_ON;
            dd_led_on(led_pin);
            Serial.printf("\n[FSM] TRANSITION: OFF -> ON\n");
            Serial.printf("  LED Status: ON (Pin %d)\n", led_pin);
            Serial.printf("  Timestamp: %lu ms\n\n", millis());
            break;

        case STATE_ON:
            current_state = STATE_OFF;
            dd_led_off(led_pin);
            Serial.printf("\n[FSM] TRANSITION: ON -> OFF\n");
            Serial.printf("  LED Status: OFF (Pin %d)\n", led_pin);
            Serial.printf("  Timestamp: %lu ms\n\n", millis());
            break;
    }
}