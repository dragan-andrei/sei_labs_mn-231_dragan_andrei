#include <Arduino.h>
#include "ctrl_stdio.h"
#include "ctrl_led.h"


#define LED_PIN 12
#define BUTTON_PIN 10
#define BUFF_LENGHT 100


void setup() {
  ctrl_stdio_init();
  ctrl_led_init();
  printf("System este init!\n");
}

void loop() {
  char str[BUFF_LENGHT] = {0};
  printf("Enter command (led_on / led_off / led_toggle): ");
  scanf("%s", str);
  ctrl_led_update(str);
}
