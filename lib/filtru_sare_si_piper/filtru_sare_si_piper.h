#ifndef FILTRU_SARE_SI_PIPER_H
#define FILTRU_SARE_SI_PIPER_H

#include <Arduino.h>

typedef struct
{
    float sample_0;
    float sample_1;
    float sample_2;
    uint8_t sample_count;
} filtru_sare_si_piper_t;

void filtru_sare_si_piper_init(filtru_sare_si_piper_t *filter);
float filtru_sare_si_piper_apply(filtru_sare_si_piper_t *filter, float sample);

#endif // FILTRU_SARE_SI_PIPER_H
