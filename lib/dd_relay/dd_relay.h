#ifndef dd_relay_h
#define dd_relay_h

#define DD_RELAY_OUTPUT 0x01

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum
{
    DD_RELAY_OFF = 0,
    DD_RELAY_ON = 1,
} dd_relay_state_t;

typedef struct
{
    uint8_t pin_number;
    uint8_t active_signal_level;
    dd_relay_state_t state;
    void (*set_state)(uint8_t, uint8_t);
    void (*set_pin_mode)(uint8_t, uint8_t);
} dd_relay_t;

void dd_relay_init(dd_relay_t *relay,
                   uint8_t pin_number,
                   uint8_t mode,
                   uint8_t active_signal_level,
                   void (*set_state)(uint8_t, uint8_t),
                   void (*set_pin_mode)(uint8_t, uint8_t));
void dd_relay_set_on(dd_relay_t *relay);
void dd_relay_set_off(dd_relay_t *relay);
void dd_relay_toggle(dd_relay_t *relay);
dd_relay_state_t dd_relay_get_state(const dd_relay_t *relay);

#endif