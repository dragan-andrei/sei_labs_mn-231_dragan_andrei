#include "ctrl_task_pid.h"
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <stdio.h>

#include "configs.h"
#include "dd_pot.h"
#include "dd_heater.h"
#include "algo_pid.h"
#include "srv_temp.h"
#include "sim_plant.h"

// Obiectele driverelor
static dd_pot_t g_pot;
static dd_heater_t g_heater;

// Obiect PID
static pid_context_t g_pid;

// Variabilă indicator de status pentru task-ul de PID
static uint8_t g_pid_module_initialized = 0;

// O funcție tip wrapper pentru analogRead/Write standard ca să nu aruncam
// structura arduino în driverele cu abstracții
static uint16_t wrap_analog_read(uint8_t pin) {
    return (uint16_t)analogRead(pin);
}

static void wrap_analog_write(uint8_t pin, int val) {
    analogWrite(pin, val);
}

static void wrap_pin_mode(uint8_t pin, uint8_t mode) {
    pinMode(pin, mode);
}

void ctrl_task_pid_init(void) {
    if (g_pid_module_initialized == 1) {
        return;
    }

    // Inițializăm potențiometrul (SetPoint: 20-50°C)
    dd_pot_init(&g_pot, POT_PIN, 20.0f, 50.0f, wrap_analog_read, wrap_pin_mode);

    // Inițializăm actuatorul (Heater PWM resolutie 8 biti, adică max 255)
    dd_heater_init(&g_heater, HEATER_PWM_PIN, 255, wrap_analog_write, wrap_pin_mode);

    // Inițializăm simularea srv_temp / plant (cod preluat de la lab)
    srv_temp_init();

    // Setăm PID-ul
    pid_init(&g_pid, PID_KP, PID_KI, PID_KD, 0.0f, 255.0f);

    g_pid_module_initialized = 1;
}

void ctrl_pid_control_loop(void *params) {
    (void)params;

    // Ne asigurăm că modulele sunt pornite
    ctrl_task_pid_init();

    // Setăm frecvența buclei OS. De obicei RTOS vTaskDelay cere Ticks.
    const TickType_t xDelay = OS_PID_TASK_DELAY_MS / portTICK_PERIOD_MS;
    float dt = OS_PID_TASK_DELAY_MS / 1000.0f; // dt in secunde pentru algoritmul PID

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;) {
        // 1. Preluarea valorii cerute (SetPoint de la Potentiometru)
        float setpoint_c = dd_pot_get_setpoint(&g_pot);

        // 2. Extragerea valorilor de mediu simulate
        float current_temp_c = srv_temp_read();

        // 3. Calcul PID pur
        float pwm_output = pid_compute(&g_pid, setpoint_c, current_temp_c, dt);

        // 4. Update-uri către hardware (prin Device Driver)
        uint8_t output_byte = (uint8_t)pwm_output;
        dd_heater_set_pwm(&g_heater, output_byte);

        // 5. Update spre modelul termic (astfel încât sistemul virtual să se încălzească)
        sim_plant_update(output_byte, dt);

        // 6. Raportare la consolă (Serial)
        printf("[PID CTRL] SP: %5.2f °C | Temp: %5.2f °C | PWM Out: %3d (%.1f%%)\n",
               setpoint_c, current_temp_c, output_byte, (output_byte / 255.0f) * 100.0f);

        // Așteptăm stabil pentru un ciclu complet și previzibil
        vTaskDelayUntil(&last_wake_time, xDelay);
    }
}