#ifndef DD_POT_HG_H
#define DD_POT_HG_H

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct 
{

    char *name;
    uint8_t pin;
    uint16_t raw_value;
    double resistance;
    double voltage;
    uint16_t max_adc_value;
    double reference_voltage;
    double min_resistance;
    double max_resistance;
    int (*analog_read)(uint8_t);
} dd_pot_hg_t;

void dd_pot_hg_init(dd_pot_hg_t *pot, 
                    const char *name, 
                    uint16_t max_adc_value,
                    double reference_voltage,
                    double min_resistance,
                    double max_resistance,
                    uint8_t pin, 
                    int (*analog_read)(uint8_t)
                );

void dd_pot_hg_update(dd_pot_hg_t *pot);

uint16_t dd_pot_hg_get_raw_value(dd_pot_hg_t *pot);
double dd_pot_hg_get_resistance(dd_pot_hg_t *pot);
double dd_pot_hg_get_voltage(dd_pot_hg_t *pot);

#endif // DD_POT_HG_H