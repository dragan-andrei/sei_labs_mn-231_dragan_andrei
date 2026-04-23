#ifndef SIM_PLANT_H
#define SIM_PLANT_H

#include <stdint.h>

void sim_plant_init(float start_temp);
void sim_plant_update(uint8_t heater_pwm, float dt_seconds);
float sim_plant_get_temp(void);

#endif