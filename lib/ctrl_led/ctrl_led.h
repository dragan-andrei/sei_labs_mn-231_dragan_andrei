#ifndef ctrl_led_h
#define ctrl_led_h

#include <ctrl_stdio.h>
#include <dd_led.h>

#define LED_PIN_NUMBER 12
#define LED_CMD_COUNT 3

typedef enum {
  ctrl_led_on = 0,
  ctrl_led_off = 1,
  ctrl_led_toggle = 2
} ctrl_led_command_t;

void ctrl_led_init(void);
void ctrl_led_update(const char* input);

#endif 