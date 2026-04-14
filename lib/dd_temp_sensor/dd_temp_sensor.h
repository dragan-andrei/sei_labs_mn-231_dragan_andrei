#ifndef DD_TEMP_SENSOR_H
#define DD_TEMP_SENSOR_H

#include <Arduino.h>
#include <stdbool.h>

#include "filtru_medie_ponderata.h"
#include "filtru_sare_si_piper.h"

typedef enum
{
	DD_TEMP_SENSOR_OK = 0,
	DD_TEMP_SENSOR_ERROR,
	DD_TEMP_SENSOR_DISCONNECTED
} dd_temp_sensor_status_t;

typedef struct
{
	uint8_t pin;
	float temperature_c;
	float last_voltage_v;
	float raw_temperature_c;
	float salt_pepper_temperature_c;
	uint16_t raw_adc_value;
	uint16_t salt_pepper_adc_value;
	uint16_t last_adc_value;
	uint8_t is_initialized;
	dd_temp_sensor_status_t status;
	filtru_sare_si_piper_t salt_pepper_filter;
	filtru_medie_ponderata_t weighted_filter;
} dd_temp_sensor_t;

void dd_temp_sensor_init(dd_temp_sensor_t *sensor, uint8_t pin);
bool dd_temp_sensor_read_celsius(dd_temp_sensor_t *sensor, float *temperature_c);
dd_temp_sensor_status_t dd_temp_sensor_get_status(const dd_temp_sensor_t *sensor);

#endif // DD_TEMP_SENSOR_H
