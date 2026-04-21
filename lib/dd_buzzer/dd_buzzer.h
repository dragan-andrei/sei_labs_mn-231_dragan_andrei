#ifndef DD_BUZZER_H
#define DD_BUZZER_H

#include <Arduino.h>
#include <stdint.h>

typedef enum
{
    DD_BUZZER_OFF = 0,
    DD_BUZZER_ON = 1
} dd_buzzer_state_t;

typedef struct
{
    uint8_t pin_number;
    dd_buzzer_state_t state;
} dd_buzzer_t;

void dd_buzzer_init(dd_buzzer_t *buzzer, uint8_t pin_number);
void dd_buzzer_beep(dd_buzzer_t *buzzer, uint16_t frequency_hz, uint16_t duration_ms);
void dd_buzzer_off(dd_buzzer_t *buzzer);
dd_buzzer_state_t dd_buzzer_get_state(const dd_buzzer_t *buzzer);

#endif // DD_BUZZER_H
