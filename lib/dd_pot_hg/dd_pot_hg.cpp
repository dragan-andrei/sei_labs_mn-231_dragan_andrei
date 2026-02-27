#include "dd_pot_hg.h"

void dd_pot_hg_init(dd_pot_hg_t *pot, 
                    const char *name, 
                    uint16_t max_adc_value,
                    double reference_voltage,
                    double min_resistance,
                    double max_resistance,
                    uint8_t pin, 
                    int (*analog_read)(uint8_t)
                ) {
    pot->name = strdup(name);
    pot->pin = pin;
    pot->raw_value = 0;
    pot->resistance = 0.0;
    pot->voltage = 0.0;
    pot->max_adc_value = max_adc_value;
    pot->reference_voltage = reference_voltage;
    pot->min_resistance = min_resistance;
    pot->max_resistance = max_resistance;
    pot->analog_read = analog_read;
}

void dd_pot_hg_update(dd_pot_hg_t *pot) {
    pot->raw_value = pot->analog_read(pot->pin);
    pot->voltage = (pot->raw_value / (double)pot->max_adc_value) * pot->reference_voltage;
    
    // Calculate resistance using voltage divider formula
    if (pot->voltage > 0) {
        double r_fixed = (pot->reference_voltage * pot->max_resistance) / (pot->reference_voltage - pot->voltage);
        pot->resistance = r_fixed;
    } else {
        pot->resistance = pot->max_resistance; // Assume max resistance if voltage is zero
    }
}

double dd_pot_hg_get_raw_value(dd_pot_hg_t *pot) {
    return pot->raw_value;
}

double dd_pot_hg_get_resistance(dd_pot_hg_t *pot) {
    return pot->resistance;
}

double dd_pot_hg_get_voltage(dd_pot_hg_t *pot) {
    return pot->voltage;
}