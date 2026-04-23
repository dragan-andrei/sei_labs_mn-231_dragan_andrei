#ifndef APP_FSM_SEMAFOR_H
#define APP_FSM_SEMAFOR_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "configs.h"

// Definirea stărilor automatului finit
typedef enum {
    ST_EST_VERDE,
    ST_EST_GALBEN,
    ST_ALL_RED_1,
    ST_NORD_VERDE,
    ST_NORD_GALBEN,
    ST_ALL_RED_2
} TrafficLightState_t;

// Funcții publice pentru a inițializa task-urile
void app_semafor_init();

#endif