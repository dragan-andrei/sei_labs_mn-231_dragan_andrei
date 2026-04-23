#include "dd_heater.h"
#include <stddef.h>

void dd_heater_init(dd_heater_t *heater, uint8_t pin, uint8_t max_pwm,
                    void (*analog_write_func)(uint8_t, int),
                    void (*pin_mode_func)(uint8_t, uint8_t)) {
    if (heater == NULL || analog_write_func == NULL || pin_mode_func == NULL) return;
    
    heater->pin = pin;
    heater->pwm_resolution = max_pwm;
    heater->analog_write_func = analog_write_func;
    heater->pin_mode_func = pin_mode_func;

    // Configurăm modul OUPUT (ox01 = 1)
    heater->pin_mode_func(heater->pin, 0x01);
}

void dd_heater_set_pwm(const dd_heater_t *heater, uint8_t pwm_value) {
    if (heater == NULL) return;
    
    // Verificăm limitele rezoluției doar ca măsură de garanție
    uint8_t output = (pwm_value > heater->pwm_resolution) ? heater->pwm_resolution : pwm_value;
    heater->analog_write_func(heater->pin, output);
}

void dd_heater_set_percentage(const dd_heater_t *heater, float percentage) {
    if (heater == NULL) return;

    if (percentage < 0.0f) percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;

    uint8_t pwm_value = (uint8_t)((percentage / 100.0f) * heater->pwm_resolution);
    dd_heater_set_pwm(heater, pwm_value);
}