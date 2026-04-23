#ifndef CONFIGS_H
#define CONFIGS_H

#include <Arduino.h>

// --- Parametrii pentru Lab 5.2 (PID) ---
#define POT_PIN A0          // Analog pin pentru SetPoint (potentiometru)
#define HEATER_PWM_PIN 3    // Pin PWM Arduino Mega pentru LED (incalzitor)

// Parametrii PID (K_p, K_i, K_d)
#define PID_KP 10.0
#define PID_KI 0.5
#define PID_KD 2.0

// Parametrii de simulare termica
#define TEMP_AMBIENT 20.0

// OS Config
#define BAUDRATE 115200

#define OS_PID_TASK_DELAY_MS 100 // Ticks delay pentru bucla in milisecunde
#define OS_PID_TASK_STACK_SIZE 256
#define OS_PID_TASK_PRIORITY 2

#endif