#ifndef dd_l298_h
#define dd_l298_h

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum
{
    DD_L298_DIRECTION_STOP = 0,
    DD_L298_DIRECTION_FORWARD,
    DD_L298_DIRECTION_BACKWARD
} dd_l298_direction_t;

typedef struct
{
    uint8_t pin_ena_pwm;
    uint8_t pin_in1;
    uint8_t pin_in2;
    uint8_t pwm_value;
    dd_l298_direction_t direction;
    void (*set_digital)(uint8_t, uint8_t);
    void (*set_pwm)(uint8_t, int);
    void (*set_pin_mode)(uint8_t, uint8_t);
} dd_l298_t;

void dd_l298_init(dd_l298_t *driver,
                  uint8_t pin_ena_pwm,
                  uint8_t pin_in1,
                  uint8_t pin_in2,
                  void (*set_digital)(uint8_t, uint8_t),
                  void (*set_pwm)(uint8_t, int),
                  void (*set_pin_mode)(uint8_t, uint8_t));

void dd_l298_set_direction(dd_l298_t *driver, dd_l298_direction_t direction);
void dd_l298_set_pwm(dd_l298_t *driver, uint8_t pwm_value);
void dd_l298_stop(dd_l298_t *driver);

#endif // dd_l298_h
