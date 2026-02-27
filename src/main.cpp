#include <Arduino.h>
#include "srv_scheduler.h"

void setup() {
  srv_scheduler_init();
}

void loop() {
  ctrl_idle_task(NULL);
}
