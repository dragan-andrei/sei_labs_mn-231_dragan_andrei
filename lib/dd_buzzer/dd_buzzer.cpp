#include "dd_buzzer.h"

void dd_buzzer_init(dd_buzzer_t *buzzer, uint8_t pin_number)
{
    if (buzzer == NULL)
    {
        return;
    }

    buzzer->pin_number = pin_number;
    buzzer->state = DD_BUZZER_OFF;

    pinMode(buzzer->pin_number, OUTPUT);
    noTone(buzzer->pin_number);
}

void dd_buzzer_beep(dd_buzzer_t *buzzer, uint16_t frequency_hz, uint16_t duration_ms)
{
    if (buzzer == NULL)
    {
        return;
    }

    tone(buzzer->pin_number, frequency_hz, duration_ms);
    buzzer->state = DD_BUZZER_ON;
}

void dd_buzzer_off(dd_buzzer_t *buzzer)
{
    if (buzzer == NULL)
    {
        return;
    }

    noTone(buzzer->pin_number);
    buzzer->state = DD_BUZZER_OFF;
}

dd_buzzer_state_t dd_buzzer_get_state(const dd_buzzer_t *buzzer)
{
    if (buzzer == NULL)
    {
        return DD_BUZZER_OFF;
    }

    return buzzer->state;
}
