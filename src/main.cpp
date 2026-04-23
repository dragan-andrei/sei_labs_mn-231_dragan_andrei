#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#include "srv_scheduler.h"

void setup() {
    // 1. Initializare globala a aplicatiei si OS
    srv_scheduler_init();
}

void loop() {
    // În arhitectura FreeRTOS bazată pe task-uri, 
    // metoda loop() nu este utilizată. 
    // Dispatch-ul este asigurat în fundal.
}