#ifndef DD_HEATER_H
#define DD_HEATER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Abstracție pentru încălzitor prin semnal PWM
typedef struct {
    uint8_t pin;
    uint8_t pwm_resolution; // 255 pentru 8 biți
    void (*analog_write_func)(uint8_t, int);
    void (*pin_mode_func)(uint8_t, uint8_t);
} dd_heater_t;

// Inițializează driver-ul de heater
void dd_heater_init(dd_heater_t *heater, uint8_t pin, uint8_t max_pwm,
                    void (*analog_write_func)(uint8_t, int),
                    void (*pin_mode_func)(uint8_t, uint8_t));

// Aplică o valoare PWM la heater
void dd_heater_set_pwm(const dd_heater_t *heater, uint8_t pwm_value);

// Aplică direct în procente (0.0f la 100.0f)
void dd_heater_set_percentage(const dd_heater_t *heater, float percentage);

#ifdef __cplusplus
}
#endif

#endif // DD_HEATER_H