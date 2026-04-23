#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// ==========================================
// CONFIGURAȚII SEMAFOR DIRECȚIA EST-VEST
// ==========================================
#define PIN_LED_EST_R   2   // Pin pentru LED Roșu Est
#define PIN_LED_EST_Y   4   // Pin pentru LED Galben Est
#define PIN_LED_EST_G   5   // Pin pentru LED Verde Est

// ==========================================
// CONFIGURAȚII SEMAFOR DIRECȚIA NORD-SUD
// ==========================================
#define PIN_LED_NORD_R  18  // Pin pentru LED Roșu Nord
#define PIN_LED_NORD_Y  19  // Pin pentru LED Galben Nord
#define PIN_LED_NORD_G  21  // Pin pentru LED Verde Nord

// ==========================================
// CONFIGURAȚII SENZORI / BUTOANE
// ==========================================
#define PIN_BTN_NORD    22  // Pin pentru Buton Cerere Nord

// ==========================================
// CONFIGURAȚII COMUNICARE SERIALĂ
// ==========================================
#define SERIAL_BAUDRATE 115200

// ==========================================
// CONFIGURAȚII TEMPORIZARE SEMAFOR (ms)
// ==========================================
#define TRAFFIC_EST_REQUEST_HOLD_MS      2000
#define TRAFFIC_EST_FAILSAFE_TIMEOUT_MS  30000
#define TRAFFIC_EST_GALBEN_MS            2000
#define TRAFFIC_ALL_RED_MS               1000
#define TRAFFIC_NORD_VERDE_MS            5000
#define TRAFFIC_NORD_GALBEN_MS           2000

// ==========================================
// CONFIGURAȚII BUTON (ms)
// ==========================================
#define BTN_POLL_MS                      50
#define BTN_DEBOUNCE_MS                  120

// ==========================================
// CONFIGURAȚII FreeRTOS
// ==========================================
#define FSM_SEMAFOR_TASK_STACK_SIZE      2048
#define FSM_SEMAFOR_BTN_TASK_PRIORITY    1
#define FSM_SEMAFOR_LOGIC_TASK_PRIORITY  2

#endif // CONFIGS_H