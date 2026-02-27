#include <Arduino.h>
#include "srv_scheduler.h"



#define LED_PIN 12
#define BUTTON_PIN 10
#define BUFF_LENGHT 100


void setup() {
  srv_scheduler_init();
}

void loop() {
  ctrl_idle_task(NULL);
}