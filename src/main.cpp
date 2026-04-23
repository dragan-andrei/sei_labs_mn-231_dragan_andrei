#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dd_button.h"
#include "dd_led.h"
#include "ctrl_stdio.h"
#include "app_fsm.h"

#include "configs.h"
#include "srv_scheduler.h"

void setup() {
    ctrl_stdio_serial_init();
    delay(500);
    
    // Inițializare hardware FSM
    app_fsm_init(MAIN_PIN_LED);
    
    ctrl_stdio_printf("\n\n=== SISTEM INIȚIALIZAT (ESP32) ===\n");
    ctrl_stdio_printf("Core RTOS: %d\n", xPortGetCoreID());
    ctrl_stdio_printf("LED Pin: %d | Button Pin: %d\n", MAIN_PIN_LED, MAIN_PIN_BTN);

    // Pornește scheduler-ul pentru task-urile FSM
    srv_scheduler_init();
}

void loop() {
    // În mediul ESP32, funcția loop() rulează ca un task FreeRTOS separat.
    // Lăsăm acest task (loopTask) să doarmă, pentru a economisi resurse,
    // deoarece logica noastră rulează în FSM_Task creat mai sus.
    vTaskDelete(NULL); 
}
