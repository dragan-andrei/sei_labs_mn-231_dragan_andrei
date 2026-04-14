#include "filtru_medie_ponderata.h"

#include "configs.h"

void filtru_medie_ponderata_init(filtru_medie_ponderata_t *filter)
{
    if (filter == NULL)
    {
        return;
    }

    filter->sample_0 = 0.0f;
    filter->sample_1 = 0.0f;
    filter->sample_2 = 0.0f;
    filter->sample_count = 0U;
}

float filtru_medie_ponderata_apply(filtru_medie_ponderata_t *filter, float sample)
{
    if (filter == NULL)
    {
        return sample;
    }

    filter->sample_0 = filter->sample_1;
    filter->sample_1 = filter->sample_2;
    filter->sample_2 = sample;

    if (filter->sample_count < 3U)
    {
        filter->sample_count++;
        return sample;
    }

    const float weight_sum = FILTRU_MEDIE_PONDERATA_WEIGHT_0
                           + FILTRU_MEDIE_PONDERATA_WEIGHT_1
                           + FILTRU_MEDIE_PONDERATA_WEIGHT_2;

    if (weight_sum <= 0.0f)
    {
        return sample;
    }

    return ((filter->sample_0 * FILTRU_MEDIE_PONDERATA_WEIGHT_0)
          + (filter->sample_1 * FILTRU_MEDIE_PONDERATA_WEIGHT_1)
          + (filter->sample_2 * FILTRU_MEDIE_PONDERATA_WEIGHT_2)) / weight_sum;
}
