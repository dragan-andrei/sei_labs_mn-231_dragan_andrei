#include "dd_l298.h"

void dd_l298_init(dd_l298_t *driver,
                  uint8_t pin_ena_pwm,
                  uint8_t pin_in1,
                  uint8_t pin_in2,
                  void (*set_digital)(uint8_t, uint8_t),
                  void (*set_pwm)(uint8_t, int),
                  void (*set_pin_mode)(uint8_t, uint8_t))
{
    if (driver == NULL || set_digital == NULL || set_pwm == NULL || set_pin_mode == NULL)
    {
        return;
    }

    driver->pin_ena_pwm = pin_ena_pwm;
    driver->pin_in1 = pin_in1;
    driver->pin_in2 = pin_in2;
    driver->pwm_value = 0U;
    driver->direction = DD_L298_DIRECTION_STOP;
    driver->set_digital = set_digital;
    driver->set_pwm = set_pwm;
    driver->set_pin_mode = set_pin_mode;

    driver->set_pin_mode(driver->pin_ena_pwm, OUTPUT);
    driver->set_pin_mode(driver->pin_in1, OUTPUT);
    driver->set_pin_mode(driver->pin_in2, OUTPUT);

    dd_l298_stop(driver);
}

void dd_l298_set_direction(dd_l298_t *driver, dd_l298_direction_t direction)
{
    if (driver == NULL || driver->set_digital == NULL)
    {
        return;
    }

    switch (direction)
    {
        case DD_L298_DIRECTION_FORWARD:
            driver->set_digital(driver->pin_in1, HIGH);
            driver->set_digital(driver->pin_in2, LOW);
            break;

        case DD_L298_DIRECTION_BACKWARD:
            driver->set_digital(driver->pin_in1, LOW);
            driver->set_digital(driver->pin_in2, HIGH);
            break;

        case DD_L298_DIRECTION_STOP:
        default:
            driver->set_digital(driver->pin_in1, LOW);
            driver->set_digital(driver->pin_in2, LOW);
            direction = DD_L298_DIRECTION_STOP;
            break;
    }

    driver->direction = direction;
}

void dd_l298_set_pwm(dd_l298_t *driver, uint8_t pwm_value)
{
    if (driver == NULL || driver->set_pwm == NULL)
    {
        return;
    }

    driver->set_pwm(driver->pin_ena_pwm, (int)pwm_value);
    driver->pwm_value = pwm_value;
}

void dd_l298_stop(dd_l298_t *driver)
{
    if (driver == NULL)
    {
        return;
    }

    dd_l298_set_pwm(driver, 0U);
    dd_l298_set_direction(driver, DD_L298_DIRECTION_STOP);
}
