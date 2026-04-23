#include "app_fsm.h"
#include "dd_led.h"
#include "ctrl_stdio.h"
#include <Arduino.h> // Pentru Serial.println

typedef enum {
    APP_FSM_STATE_OFF = 0,
    APP_FSM_STATE_ON
} app_fsm_state_t;

static app_fsm_state_t g_current_state = APP_FSM_STATE_OFF;

void app_fsm_init(uint8_t led_pin) {
    dd_led_init(led_pin);
    g_current_state = APP_FSM_STATE_OFF;
    ctrl_stdio_printf("[FSM INIT] State = 0 (OFF), LED Pin = %d\n", led_pin); 
}

void app_fsm_update(bool button_event, uint8_t led_pin) {
    if (!button_event) return; 

    switch (g_current_state) {
        case APP_FSM_STATE_OFF:
            g_current_state = APP_FSM_STATE_ON;
            dd_led_on(led_pin);
            ctrl_stdio_printf("\n[FSM] TRANSITION: OFF -> ON\n");
            ctrl_stdio_printf("  LED Status: ON (Pin %d)\n", led_pin);
            ctrl_stdio_printf("  Timestamp: %lu ms\n\n", millis());
            break;

        case APP_FSM_STATE_ON:
            g_current_state = APP_FSM_STATE_OFF;
            dd_led_off(led_pin);
            ctrl_stdio_printf("\n[FSM] TRANSITION: ON -> OFF\n");
            ctrl_stdio_printf("  LED Status: OFF (Pin %d)\n", led_pin);
            ctrl_stdio_printf("  Timestamp: %lu ms\n\n", millis());
            break;
    }
}