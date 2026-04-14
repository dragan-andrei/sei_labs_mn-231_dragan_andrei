#include "filtru_sare_si_piper.h"

static float filtru_sare_si_piper_median_of_three(float a, float b, float c)
{
    if (((a <= b) && (b <= c)) || ((c <= b) && (b <= a)))
    {
        return b;
    }

    if (((b <= a) && (a <= c)) || ((c <= a) && (a <= b)))
    {
        return a;
    }

    return c;
}

void filtru_sare_si_piper_init(filtru_sare_si_piper_t *filter)
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

float filtru_sare_si_piper_apply(filtru_sare_si_piper_t *filter, float sample)
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

    return filtru_sare_si_piper_median_of_three(filter->sample_0,
                                                 filter->sample_1,
                                                 filter->sample_2);
}
