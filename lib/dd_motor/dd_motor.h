#ifndef dd_motor_h
#define dd_motor_h

#include <stdint.h>
#include <stdlib.h>

#include "configs.h"
#include "dd_l298.h"

typedef enum
{
    DD_MOTOR_DIRECTION_STOP = 0,
    DD_MOTOR_DIRECTION_FORWARD,
    DD_MOTOR_DIRECTION_BACKWARD
} dd_motor_direction_t;

typedef struct
{
    int8_t power_percent;
    uint8_t pwm_value;
    dd_motor_direction_t direction;
} dd_motor_state_t;

typedef struct
{
    dd_l298_t l298;
    dd_motor_state_t state;
    uint8_t pwm_min;
    uint8_t pwm_max;
} dd_motor_t;

void dd_motor_init(dd_motor_t *motor,
                   uint8_t pin_ena_pwm,
                   uint8_t pin_in1,
                   uint8_t pin_in2,
                   uint8_t pwm_min,
                   uint8_t pwm_max,
                   void (*set_digital)(uint8_t, uint8_t),
                   void (*set_pwm)(uint8_t, int),
                   void (*set_pin_mode)(uint8_t, uint8_t));

void dd_motor_set_power_percent(dd_motor_t *motor, int8_t power_percent);
void dd_motor_stop(dd_motor_t *motor);
dd_motor_state_t dd_motor_get_state(const dd_motor_t *motor);

#endif // dd_motor_h
