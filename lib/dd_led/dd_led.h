#ifndef dd_led_h
#define dd_led_h

#include <Arduino.h>
#include <stdlib.h>

typedef enum {
  dd_led_off = 0,
  dd_led_on = 1,
} dd_led_state_t;

typedef struct {
  uint8_t pin_number;
  dd_led_state_t state;
  void (*set_state)(uint8_t, uint8_t);
} dd_led_t;

void dd_led_init(dd_led_t* led, uint8_t pin_number, void (*set_state_fn)(uint8_t, uint8_t));
void dd_led_set_on(dd_led_t* led);
void dd_led_set_off(dd_led_t* led);
void dd_led_toggle(dd_led_t* led);

#endif