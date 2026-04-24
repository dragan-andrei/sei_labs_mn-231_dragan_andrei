#ifndef APP_I2C_MASTER_H
#define APP_I2C_MASTER_H

#include <Arduino.h>

// Inițializează periferia I²C ca master, LED-ul de alertă și pornește
// task-ul FreeRTOS de interogare periodică a slave-ului.
void app_i2c_master_init();

#endif // APP_I2C_MASTER_H
