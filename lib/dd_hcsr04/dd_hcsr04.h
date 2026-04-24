#ifndef DD_HCSR04_H
#define DD_HCSR04_H

#include <Arduino.h>

// Structură de descriere a unui senzor HC-SR04 (pini TRIG și ECHO)
typedef struct {
    uint8_t trig_pin;
    uint8_t echo_pin;
} hcsr04_sensor_t;

// Inițializează pinii unui senzor (TRIG ca OUTPUT, ECHO ca INPUT)
void dd_hcsr04_init(const hcsr04_sensor_t* sensor);

// Declanșează o măsurătoare și returnează distanța în centimetri.
// În caz de eroare (fără ecou / distanță în afara limitelor) returnează
// HCSR04_DISTANCE_ERROR definit în include/configs.h (0xFFFF).
uint16_t dd_hcsr04_read_cm(const hcsr04_sensor_t* sensor);

#endif // DD_HCSR04_H
