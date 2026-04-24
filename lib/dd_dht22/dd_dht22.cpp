#include "dd_dht22.h"

#include <DHT.h>
#include <math.h>

#include "configs.h"
#include "ctrl_stdio.h"

namespace {
// DHT-ul trebuie construit dinamic pentru a putea primi pinul la runtime.
DHT* g_dht = nullptr;
}  // namespace

void dd_dht22_init(uint8_t data_pin, uint8_t sensor_type) {
    if (g_dht != nullptr) {
        delete g_dht;
    }
    g_dht = new DHT(data_pin, sensor_type);
    g_dht->begin();
    ctrl_stdio_printf("[DHT22] init pe GPIO %u (type=%u)\n",
                      static_cast<unsigned>(data_pin),
                      static_cast<unsigned>(sensor_type));
}

void dd_dht22_read(dd_dht22_sample_t* out) {
    if (out == nullptr) {
        return;
    }
    out->valid = false;
    out->temperature_c    = NAN;
    out->humidity_percent = NAN;
    if (g_dht == nullptr) {
        return;
    }

    const float h = g_dht->readHumidity();
    const float t = g_dht->readTemperature();
    if (isnan(h) || isnan(t)) {
        ctrl_stdio_print_text("[DHT22][WARN] Citire NaN\n");
        return;
    }
    if (t < DHT22_MIN_TEMPERATURE_C || t > DHT22_MAX_TEMPERATURE_C ||
        h < DHT22_MIN_HUMIDITY_PERCENT || h > DHT22_MAX_HUMIDITY_PERCENT) {
        ctrl_stdio_printf(
            "[DHT22][WARN] Citire în afara intervalului: t=%.2f h=%.2f\n",
            static_cast<double>(t),
            static_cast<double>(h));
        return;
    }
    out->temperature_c    = t;
    out->humidity_percent = h;
    out->valid            = true;
}
