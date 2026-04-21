#ifndef CTRL_HYSTERESIS_H
#define CTRL_HYSTERESIS_H

#include <stdint.h>
#include <stdbool.h>

#include "configs.h"

typedef enum
{
    CTRL_HYSTERESIS_STATE_OFF = 0,
    CTRL_HYSTERESIS_STATE_ON
} ctrl_hysteresis_state_t;

typedef struct
{
    float set_point_c;
    float hysteresis_c;
    float set_point_min_c;
    float set_point_max_c;
    float set_point_step_c;
    ctrl_hysteresis_state_t output_state;
} ctrl_hysteresis_t;

void ctrl_hysteresis_init(ctrl_hysteresis_t *ctrl,
                          float set_point_c,
                          float hysteresis_c,
                          float set_point_min_c,
                          float set_point_max_c,
                          float set_point_step_c);

ctrl_hysteresis_state_t ctrl_hysteresis_update(ctrl_hysteresis_t *ctrl,
                                               float current_temperature_c);

void ctrl_hysteresis_increment_setpoint(ctrl_hysteresis_t *ctrl);
void ctrl_hysteresis_decrement_setpoint(ctrl_hysteresis_t *ctrl);

float ctrl_hysteresis_get_setpoint(const ctrl_hysteresis_t *ctrl);
float ctrl_hysteresis_get_low_threshold(const ctrl_hysteresis_t *ctrl);
float ctrl_hysteresis_get_high_threshold(const ctrl_hysteresis_t *ctrl);
ctrl_hysteresis_state_t ctrl_hysteresis_get_state(const ctrl_hysteresis_t *ctrl);

#endif // CTRL_HYSTERESIS_H
