#ifndef DD_POT_H
#define DD_POT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Structura driver-ului pentru potentirometru
typedef struct {
    uint8_t pin;
    float min_val_c;
    float max_val_c;
    uint16_t (*analog_read_func)(uint8_t);
    void (*pin_mode_func)(uint8_t, uint8_t);
} dd_pot_t;

// Inițializarea driverului
void dd_pot_init(dd_pot_t *pot, uint8_t pin, float min_c, float max_c, 
                 uint16_t (*analog_read_func)(uint8_t), 
                 void (*pin_mode_func)(uint8_t, uint8_t));

// Returnează valoarea scalată în grade Celsius (SetPoint)
float dd_pot_get_setpoint(const dd_pot_t *pot);

#ifdef __cplusplus
}
#endif

#endif // DD_POT_H