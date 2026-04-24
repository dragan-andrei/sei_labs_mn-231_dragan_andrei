#ifndef APP_I2C_MASTER_H
#define APP_I2C_MASTER_H

#include <Arduino.h>
#include <Wire.h>

// Inițializează `bus` ca I²C master pe pinii (sda_pin, scl_pin), configurează
// LED-ul de alertă și pornește task-ul FreeRTOS care interoghează `slave_address`.
//
// `bus` trebuie să fie un pointer valid pentru toată durata de viață a aplicației
// (ex.: `&Wire` sau `&Wire1`).
void app_i2c_master_init(TwoWire* bus,
                         uint8_t sda_pin,
                         uint8_t scl_pin,
                         uint8_t slave_address);

#endif // APP_I2C_MASTER_H
