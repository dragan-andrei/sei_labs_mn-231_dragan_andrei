#include "sim_plant.h"
#include "configs.h"

static float current_temp;

void sim_plant_init(float start_temp) {
    current_temp = start_temp;
}

void sim_plant_update(uint8_t heater_pwm, float dt_seconds) {
    // Factor de incalzire (puterea incalzitorului aplicata in timp)
    float heat_added = (heater_pwm / 255.0) * 5.0 * dt_seconds; 
    
    // Factor de racire (pierderea de caldura spre ambient)
    float heat_lost = (current_temp - TEMP_AMBIENT) * 0.1 * dt_seconds;
    
    current_temp = current_temp + heat_added - heat_lost;
}

float sim_plant_get_temp(void) {
    return current_temp;
}