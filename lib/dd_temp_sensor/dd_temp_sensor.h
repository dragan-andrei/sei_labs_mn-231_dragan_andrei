#ifndef DD_TEMP_SENSOR_H
#define DD_TEMP_SENSOR_H

#include <Arduino.h>
#include <stdbool.h>

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
	uint8_t is_initialized;
	dd_temp_sensor_status_t status;
	void *one_wire_handle;
	void *dallas_handle;
} dd_temp_sensor_t;

void dd_temp_sensor_init(dd_temp_sensor_t *sensor, uint8_t pin);
bool dd_temp_sensor_read_celsius(dd_temp_sensor_t *sensor, float *temperature_c);
dd_temp_sensor_status_t dd_temp_sensor_get_status(const dd_temp_sensor_t *sensor);

#endif // DD_TEMP_SENSOR_H
