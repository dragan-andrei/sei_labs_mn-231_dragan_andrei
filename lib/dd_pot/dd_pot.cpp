#include "dd_pot.h"
#include <stddef.h>

#define POT_ADC_MAX_VALUE 1023.0f

void dd_pot_init(dd_pot_t *pot, uint8_t pin, float min_c, float max_c, 
                 uint16_t (*analog_read_func)(uint8_t), 
                 void (*pin_mode_func)(uint8_t, uint8_t)) {
    if (pot == NULL || analog_read_func == NULL || pin_mode_func == NULL) {
        return;
    }
    
    pot->pin = pin;
    pot->min_val_c = min_c;
    pot->max_val_c = max_c;
    pot->analog_read_func = analog_read_func;
    pot->pin_mode_func = pin_mode_func;

    // Configurare pin ca INPUT (0x00 în Arduino, o punem fix pe 0 pt portabilitate)
    pot->pin_mode_func(pot->pin, 0x00);
}

float dd_pot_get_setpoint(const dd_pot_t *pot) {
    if (pot == NULL || pot->analog_read_func == NULL) {
        return 0.0f;
    }

    uint16_t raw_val = pot->analog_read_func(pot->pin);
    float interval = pot->max_val_c - pot->min_val_c;
    
    return pot->min_val_c + ( (float)raw_val / POT_ADC_MAX_VALUE ) * interval;
}