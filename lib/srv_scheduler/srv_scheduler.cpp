#include "srv_scheduler.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "configs.h"
#include "dd_button.h"
#include "app_fsm.h"
#include "ctrl_stdio.h"

// Handle pentru task
static TaskHandle_t g_fsm_task_handle;

// Obiectul butonului care include starea de debounce
static dd_button_t g_main_button;

// Task dedicat pentru Button & FSM
static void srv_scheduler_fsm_task(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(FSM_TASK_DELAY_MS); // Rulare la fiecare 10 ms

    for (;;) {
        // 1. Folosim driver-ul de buton pentru a citi și filtra starea (Debounce integrat)
        bool trigger_event = dd_button_update(&g_main_button, millis());

        if (trigger_event) {
            ctrl_stdio_printf("[BUTTON] Valid press detected (ESP32 Core %d)!\n", xPortGetCoreID());
        }

        // 4. Apelăm FSM
        app_fsm_update(trigger_event, MAIN_PIN_LED);

        // 5. Yield către RTOS
        vTaskDelay(xDelay);
    }
}

void srv_scheduler_init(void) {
    // Inițializare hardware buton controlat de scheduler
    dd_button_init(&g_main_button, MAIN_PIN_BTN, DEBOUNCE_DELAY_MS);

    // Creăm un task hardware dedicat pe nucleul 1 al ESP32
    xTaskCreatePinnedToCore(
        srv_scheduler_fsm_task, // Funcția task-ului
        "FSM_Task",             // Numele task-ului
        FSM_TASK_STACK_SIZE,    // Dimensiunea stivei
        NULL,                   // Parametri
        FSM_TASK_PRIORITY,      // Prioritate
        &g_fsm_task_handle,     // Handle
        FSM_TASK_CORE           // Rulare pe Core
    );
}
