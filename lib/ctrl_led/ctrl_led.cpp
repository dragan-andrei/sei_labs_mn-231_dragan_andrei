#include "ctrl_led.h"
#include <string.h>

dd_led_t led;

static char const *commands[] = {
  "led_on",
  "led_off",
  "led_toggle"
};

static void (*led_set_state_function[LED_CMD_COUNT])(dd_led_t*) = {
  dd_led_set_on,
  dd_led_set_off,
  dd_led_toggle
};

void ctrl_led_init(void)
{
    pinMode(LED_PIN_NUMBER, OUTPUT);
    dd_led_init(&led, LED_PIN_NUMBER, digitalWrite);
}

void ctrl_led_update(const char* input)
{
    for (int i = 0; i < LED_CMD_COUNT; i++) {
        if (strcmp(input, commands[i]) == 0) {
            led_set_state_function[i](&led);
            printf("OK: %s\n", commands[i]);
            return;
        }
    }
    printf("Unknown command: %s\n", input);
    printf("Available: led_on, led_off, led_toggle\n");
}