#include "ctrl_hysteresis.h"

#include <stddef.h>

void ctrl_hysteresis_init(ctrl_hysteresis_t *ctrl,
                          float set_point_c,
                          float hysteresis_c,
                          float set_point_min_c,
                          float set_point_max_c,
                          float set_point_step_c)
{
    if (ctrl == NULL)
    {
        return;
    }

    ctrl->set_point_c = set_point_c;
    ctrl->hysteresis_c = hysteresis_c;
    ctrl->set_point_min_c = set_point_min_c;
    ctrl->set_point_max_c = set_point_max_c;
    ctrl->set_point_step_c = set_point_step_c;
    ctrl->output_state = CTRL_HYSTERESIS_STATE_OFF;
}

ctrl_hysteresis_state_t ctrl_hysteresis_update(ctrl_hysteresis_t *ctrl,
                                               float current_temperature_c)
{
    if (ctrl == NULL)
    {
        return CTRL_HYSTERESIS_STATE_OFF;
    }

    const float low_threshold = ctrl->set_point_c - ctrl->hysteresis_c;
    const float high_threshold = ctrl->set_point_c + ctrl->hysteresis_c;

    if (ctrl->output_state == CTRL_HYSTERESIS_STATE_OFF)
    {
        /* Heater ON when temperature drops below low threshold */
        if (current_temperature_c < low_threshold)
        {
            ctrl->output_state = CTRL_HYSTERESIS_STATE_ON;
        }
    }
    else
    {
        /* Heater OFF when temperature rises above high threshold */
        if (current_temperature_c >= high_threshold)
        {
            ctrl->output_state = CTRL_HYSTERESIS_STATE_OFF;
        }
    }

    return ctrl->output_state;
}

void ctrl_hysteresis_increment_setpoint(ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return;
    }

    const float new_setpoint = ctrl->set_point_c + ctrl->set_point_step_c;

    if (new_setpoint <= ctrl->set_point_max_c)
    {
        ctrl->set_point_c = new_setpoint;
    }
}

void ctrl_hysteresis_decrement_setpoint(ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return;
    }

    const float new_setpoint = ctrl->set_point_c - ctrl->set_point_step_c;

    if (new_setpoint >= ctrl->set_point_min_c)
    {
        ctrl->set_point_c = new_setpoint;
    }
}

float ctrl_hysteresis_get_setpoint(const ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return HYSTERESIS_DEFAULT_SETPOINT_C;
    }

    return ctrl->set_point_c;
}

float ctrl_hysteresis_get_low_threshold(const ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return HYSTERESIS_DEFAULT_SETPOINT_C - HYSTERESIS_BAND_C;
    }

    return ctrl->set_point_c - ctrl->hysteresis_c;
}

float ctrl_hysteresis_get_high_threshold(const ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return HYSTERESIS_DEFAULT_SETPOINT_C + HYSTERESIS_BAND_C;
    }

    return ctrl->set_point_c + ctrl->hysteresis_c;
}

ctrl_hysteresis_state_t ctrl_hysteresis_get_state(const ctrl_hysteresis_t *ctrl)
{
    if (ctrl == NULL)
    {
        return CTRL_HYSTERESIS_STATE_OFF;
    }

    return ctrl->output_state;
}
