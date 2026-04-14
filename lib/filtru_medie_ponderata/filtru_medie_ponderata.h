#ifndef FILTRU_MEDIE_PONDERATA_H
#define FILTRU_MEDIE_PONDERATA_H

#include <Arduino.h>

typedef struct
{
    float sample_0;
    float sample_1;
    float sample_2;
    uint8_t sample_count;
} filtru_medie_ponderata_t;

void filtru_medie_ponderata_init(filtru_medie_ponderata_t *filter);
float filtru_medie_ponderata_apply(filtru_medie_ponderata_t *filter, float sample);

#endif // FILTRU_MEDIE_PONDERATA_H
