#ifndef APP_I2C_SLAVE_H
#define APP_I2C_SLAVE_H

#include <Arduino.h>
#include <Wire.h>
#include <stddef.h>

// Inițializează senzorii HC-SR04, configurează `bus` ca I²C slave pe pinii
// (sda_pin, scl_pin) cu adresa `slave_address` și pornește task-urile
// FreeRTOS (achiziție + reîmprospătare buffer). Dacă `bus == nullptr`, se sare
// peste inițializarea I²C hardware — util pentru mediul de demo unde protocolul
// e expus direct prin `app_i2c_slave_peek_response`.
//
// `bus` trebuie să fie un pointer valid pentru toată durata de viață a aplicației
// (ex.: `&Wire` sau `&Wire1`).
void app_i2c_slave_init(TwoWire* bus,
                        uint8_t sda_pin,
                        uint8_t scl_pin,
                        uint8_t slave_address);

// Expune buffer-ul cel mai recent (deja encodat ca HEAD/LEN/PAYLOAD/CHECKSUM)
// către un consumator in-process — folosit de build-ul de DEMO ca "magistrală
// virtuală" atunci când simulatorul nu poate furniza un I²C slave real.
// Copiază cel mult `capacity` octeți și returnează lungimea reală. Returnează
// 0 dacă încă nu există un pachet gata sau dacă mutex-ul nu poate fi preluat.
size_t app_i2c_slave_peek_response(uint8_t* out, size_t capacity);

#endif // APP_I2C_SLAVE_H
