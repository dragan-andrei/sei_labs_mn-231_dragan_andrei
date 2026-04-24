#ifndef APP_I2C_SLAVE_H
#define APP_I2C_SLAVE_H

#include <Arduino.h>
#include <Wire.h>

// Inițializează senzorii HC-SR04, configurează `bus` ca I²C slave pe pinii
// (sda_pin, scl_pin) cu adresa `slave_address` și pornește task-urile
// FreeRTOS (achiziție + reîmprospătare buffer).
//
// `bus` trebuie să fie un pointer valid pentru toată durata de viață a aplicației
// (ex.: `&Wire` sau `&Wire1`).
void app_i2c_slave_init(TwoWire* bus,
                        uint8_t sda_pin,
                        uint8_t scl_pin,
                        uint8_t slave_address);

#endif // APP_I2C_SLAVE_H
