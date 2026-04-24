#ifndef APP_I2C_SLAVE_H
#define APP_I2C_SLAVE_H

#include <Arduino.h>

// Inițializează periferia I²C ca slave, senzorii HC-SR04 și pornește
// task-urile FreeRTOS (achiziție senzor + reîmprospătare buffer de răspuns).
void app_i2c_slave_init();

#endif // APP_I2C_SLAVE_H
