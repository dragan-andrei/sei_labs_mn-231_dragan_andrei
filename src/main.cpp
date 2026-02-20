#include <Arduino.h>
#include "ctrl_interfone.h"



#define LED_PIN 12
#define BUTTON_PIN 10
#define BUFF_LENGHT 100


void setup() {
  ctrl_interfone_init();
}

void loop() {
 ctrl_interfone_update();
}