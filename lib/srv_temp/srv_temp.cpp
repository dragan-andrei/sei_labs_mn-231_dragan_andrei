#include "srv_temp.h"
#include "configs.h"
#include "sim_plant.h"

void srv_temp_init(void) {
    // Initializam planta virtuala la temperatura camerei
    sim_plant_init(TEMP_AMBIENT); 
}

float srv_temp_read(void) {
    // In loc de dht.readTemperature(), citim simularea
    return sim_plant_get_temp(); 
}