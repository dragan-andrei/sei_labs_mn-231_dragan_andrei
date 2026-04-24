#ifndef DD_DHT22_H
#define DD_DHT22_H

#include <stdint.h>

// ============================================================================
//  dd_dht22 — HAL peste biblioteca `DHT sensor library` (Adafruit).
// ============================================================================

typedef struct {
    float temperature_c;
    float humidity_percent;
    bool  valid;
} dd_dht22_sample_t;

// Inițializează senzorul pe `data_pin`. Apelează `.begin()` pe biblioteca DHT.
void dd_dht22_init(uint8_t data_pin, uint8_t sensor_type);

// Efectuează o citire blocantă (~250 ms tipic pentru DHT22). Raportează
// valorile în `out`. Setează `out->valid = false` dacă citirea e NaN sau în
// afara intervalului rezonabil definit în configs.h.
void dd_dht22_read(dd_dht22_sample_t* out);

#endif // DD_DHT22_H
