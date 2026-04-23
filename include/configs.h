#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// ==========================================
// Configurări pentru ecranul LCD (I2C)
// ==========================================
#define LCD_COLS 16  // Numărul de coloane ale LCD-ului (ex: 16 sau 20)
#define LCD_ROWS 2   // Numărul de rânduri ale LCD-ului (ex: 2 sau 4)

// ==========================================
// Configurări pentru Tastatura Matricială (Keypad)
// ==========================================
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// Pinii la care sunt conectate RÂNDURILE tastaturii
#define KEYPAD_PIN_1 13
#define KEYPAD_PIN_2 12
#define KEYPAD_PIN_3 14
#define KEYPAD_PIN_4 27

// Pinii la care sunt conectate COLOANELE tastaturii
#define KEYPAD_PIN_5 26
#define KEYPAD_PIN_6 25
#define KEYPAD_PIN_7 33
#define KEYPAD_PIN_8 32

// ==========================================
// Configurări Hardware de Bază (FSM, LED, Buton)
// ==========================================
#define MAIN_PIN_LED 2
#define MAIN_PIN_BTN 4

#define DEBOUNCE_DELAY_MS 50

// ==========================================
// Configurări FreeRTOS
// ==========================================
#define FSM_TASK_STACK_SIZE 2048
#define FSM_TASK_PRIORITY   1
#define FSM_TASK_CORE       1
#define FSM_TASK_DELAY_MS   10

#endif // CONFIGS_H