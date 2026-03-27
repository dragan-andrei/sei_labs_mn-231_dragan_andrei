#include "dd_temp_sensor.h"

#include <OneWire.h>
#include <DallasTemperature.h>

void dd_temp_sensor_init(dd_temp_sensor_t *sensor, uint8_t pin)
{
	if (sensor == NULL)
	{
		return;
	}

	sensor->pin = pin;
	sensor->temperature_c = 0.0f;
	sensor->is_initialized = 0;
	sensor->status = DD_TEMP_SENSOR_ERROR;
	sensor->one_wire_handle = NULL;
	sensor->dallas_handle = NULL;

	OneWire *one_wire = new OneWire(pin);
	if (one_wire == NULL)
	{
		return;
	}

	DallasTemperature *dallas = new DallasTemperature(one_wire);
	if (dallas == NULL)
	{
		delete one_wire;
		return;
	}

	dallas->begin();

	sensor->one_wire_handle = one_wire;
	sensor->dallas_handle = dallas;
	sensor->is_initialized = 1;
	sensor->status = (dallas->getDeviceCount() > 0U)
					 ? DD_TEMP_SENSOR_OK
					 : DD_TEMP_SENSOR_DISCONNECTED;
}

bool dd_temp_sensor_read_celsius(dd_temp_sensor_t *sensor, float *temperature_c)
{
	if (sensor == NULL || sensor->is_initialized == 0U || sensor->dallas_handle == NULL)
	{
		return false;
	}

	DallasTemperature *dallas = static_cast<DallasTemperature *>(sensor->dallas_handle);

	dallas->requestTemperatures();
	const float sampled_temperature = dallas->getTempCByIndex(0);

	if (sampled_temperature == DEVICE_DISCONNECTED_C)
	{
		sensor->status = DD_TEMP_SENSOR_DISCONNECTED;
		return false;
	}

	sensor->temperature_c = sampled_temperature;
	sensor->status = DD_TEMP_SENSOR_OK;

	if (temperature_c != NULL)
	{
		*temperature_c = sampled_temperature;
	}

	return true;
}

dd_temp_sensor_status_t dd_temp_sensor_get_status(const dd_temp_sensor_t *sensor)
{
	if (sensor == NULL)
	{
		return DD_TEMP_SENSOR_ERROR;
	}

	return sensor->status;
}
