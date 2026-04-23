#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "dd_button.h"
#include "dd_led.h"
#include "ctrl_stdio.h"
#include "app_fsm.h"

// Definește pinii conform diagram.json (Wokwi esp32-devkit-v1)
#define PIN_LED 2
#define PIN_BTN 4

// Handle pentru task
TaskHandle_t FsmTaskHandle;

// Variabile globale pentru filtrarea zgomotului de la buton (Debounce)
unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 50; // Filtru de 50ms
bool last_btn_read = false;
bool btn_stable_state = false;

// Task dedicat pentru Button & FSM
void vFsmTask(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(10); // Rulare la fiecare 10 ms

    for (;;) {
        // 1. Citim starea fizică a pinului
        bool current_btn_read = dd_button_is_pressed(PIN_BTN);
        bool trigger_event = false;

        // 2. Resetăm timer-ul dacă zgomotul mecanic schimbă starea
        if (current_btn_read != last_btn_read) {
            last_debounce_time = millis();
        }

        // 3. Dacă starea citită a rămas constantă peste 50ms, o validăm
        if ((millis() - last_debounce_time) > debounce_delay) {
            if (current_btn_read != btn_stable_state) {
                btn_stable_state = current_btn_read;
                
                // Generăm eveniment doar la apăsare
                if (btn_stable_state == true) {
                    trigger_event = true;
                    Serial.printf("[BUTTON] Valid press detected (ESP32 Core %d)!\n", xPortGetCoreID());
                }
            }
        }
        last_btn_read = current_btn_read;

        // 4. Apelăm FSM
        app_fsm_update(trigger_event, PIN_LED);

        // 5. Yield către RTOS
        vTaskDelay(xDelay);
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);
    
    // Inițializare hardware
    dd_button_init(PIN_BTN);
    app_fsm_init(PIN_LED);
    
    Serial.printf("\n\n=== SISTEM INIȚIALIZAT (ESP32) ===\n");
    Serial.printf("Core RTOS: %d\n", xPortGetCoreID());
    Serial.printf("LED Pin: %d | Button Pin: %d\n", PIN_LED, PIN_BTN);

    // Creăm un task hardware dedicat pe nucleul 1 al ESP32 (Nucleul 0 este de obicei pentru Wi-Fi/Radio)
    xTaskCreatePinnedToCore(
        vFsmTask,       // Funcția task-ului
        "FSM_Task",     // Numele task-ului
        2048,           // Dimensiunea stivei
        NULL,           // Parametri
        1,              // Prioritate (1 e ok pentru UI)
        &FsmTaskHandle, // Handle
        1               // Rulare pe Core 1
    );
}

void loop() {
    // În mediul ESP32, funcția loop() rulează ca un task FreeRTOS separat.
    // Lăsăm acest task (loopTask) să doarmă, pentru a economisi resurse,
    // deoarece logica noastră rulează în FSM_Task creat mai sus.
    vTaskDelete(NULL); 
}
