#include "dd_motor.h"

static int8_t dd_motor_clamp_power_percent(int16_t power_percent)
{
    if (power_percent > MOTOR_POWER_MAX_PERCENT)
    {
        return MOTOR_POWER_MAX_PERCENT;
    }

    if (power_percent < MOTOR_POWER_MIN_PERCENT)
    {
        return MOTOR_POWER_MIN_PERCENT;
    }

    return (int8_t)power_percent;
}

static uint8_t dd_motor_percent_to_pwm(const dd_motor_t *motor, uint8_t magnitude_percent)
{
    if (motor == NULL || magnitude_percent == 0U)
    {
        return 0U;
    }

    if (magnitude_percent >= (uint8_t)MOTOR_POWER_MAX_PERCENT)
    {
        return motor->pwm_max;
    }

    if (motor->pwm_max <= motor->pwm_min)
    {
        return motor->pwm_max;
    }

    return (uint8_t)(motor->pwm_min +
                     (((uint16_t)magnitude_percent * (uint16_t)(motor->pwm_max - motor->pwm_min)) /
                      (uint16_t)MOTOR_POWER_MAX_PERCENT));
}

void dd_motor_init(dd_motor_t *motor,
                   uint8_t pin_ena_pwm,
                   uint8_t pin_in1,
                   uint8_t pin_in2,
                   uint8_t pwm_min,
                   uint8_t pwm_max,
                   void (*set_digital)(uint8_t, uint8_t),
                   void (*set_pwm)(uint8_t, int),
                   void (*set_pin_mode)(uint8_t, uint8_t))
{
    if (motor == NULL)
    {
        return;
    }

    motor->pwm_min = pwm_min;
    motor->pwm_max = pwm_max;
    motor->state.power_percent = MOTOR_POWER_DEFAULT_PERCENT;
    motor->state.pwm_value = 0U;
    motor->state.direction = DD_MOTOR_DIRECTION_STOP;

    dd_l298_init(&motor->l298,
                 pin_ena_pwm,
                 pin_in1,
                 pin_in2,
                 set_digital,
                 set_pwm,
                 set_pin_mode);

    dd_motor_stop(motor);
}

void dd_motor_set_power_percent(dd_motor_t *motor, int8_t power_percent)
{
    uint8_t magnitude = 0U;
    int16_t absolute_power = 0;

    if (motor == NULL)
    {
        return;
    }

    motor->state.power_percent = dd_motor_clamp_power_percent((int16_t)power_percent);

    if (motor->state.power_percent > 0)
    {
        motor->state.direction = DD_MOTOR_DIRECTION_FORWARD;
        dd_l298_set_direction(&motor->l298, DD_L298_DIRECTION_FORWARD);
    }
    else if (motor->state.power_percent < 0)
    {
        motor->state.direction = DD_MOTOR_DIRECTION_BACKWARD;
        dd_l298_set_direction(&motor->l298, DD_L298_DIRECTION_BACKWARD);
    }
    else
    {
        dd_motor_stop(motor);
        return;
    }

    absolute_power = (motor->state.power_percent < 0)
                         ? (int16_t)(-motor->state.power_percent)
                         : (int16_t)motor->state.power_percent;

    magnitude = (uint8_t)absolute_power;

    motor->state.pwm_value = dd_motor_percent_to_pwm(motor, magnitude);
    dd_l298_set_pwm(&motor->l298, motor->state.pwm_value);
}

void dd_motor_stop(dd_motor_t *motor)
{
    if (motor == NULL)
    {
        return;
    }

    motor->state.power_percent = MOTOR_POWER_DEFAULT_PERCENT;
    motor->state.pwm_value = 0U;
    motor->state.direction = DD_MOTOR_DIRECTION_STOP;

    dd_l298_stop(&motor->l298);
}

dd_motor_state_t dd_motor_get_state(const dd_motor_t *motor)
{
    dd_motor_state_t state_snapshot = {MOTOR_POWER_DEFAULT_PERCENT, 0U, DD_MOTOR_DIRECTION_STOP};

    if (motor == NULL)
    {
        return state_snapshot;
    }

    state_snapshot = motor->state;
    return state_snapshot;
}
