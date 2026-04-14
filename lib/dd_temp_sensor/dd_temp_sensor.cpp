#include "dd_temp_sensor.h"

#include <math.h>

#include "configs.h"

static float dd_temp_sensor_clamp(float value, float min_value, float max_value)
{
	if (value < min_value)
	{
		return min_value;
	}

	if (value > max_value)
	{
		return max_value;
	}

	return value;
}

static uint16_t dd_temp_sensor_adc_to_u16(float adc_value)
{
	const float clamped_adc = dd_temp_sensor_clamp(adc_value,
												  NTC_ADC_MIN_VALUE,
												  NTC_ADC_MAX_VALUE_SAFE);

	return (uint16_t)(clamped_adc + SENSOR_ADC_ROUNDING_OFFSET);
}

static bool dd_temp_sensor_adc_to_voltage(float adc_value, float *voltage_v)
{
	if (voltage_v == NULL)
	{
		return false;
	}

	const float clamped_adc = dd_temp_sensor_clamp(adc_value,
													  NTC_ADC_MIN_VALUE,
													  NTC_ADC_MAX_VALUE_SAFE);

	*voltage_v = (clamped_adc * NTC_REFERENCE_VOLTAGE_V) / NTC_ADC_MAX_VALUE;
	return true;
}

static bool dd_temp_sensor_voltage_to_celsius(float voltage_v, float *temperature_c)
{
	if (temperature_c == NULL)
	{
		return false;
	}

	if (voltage_v <= 0.0f || voltage_v >= NTC_REFERENCE_VOLTAGE_V)
	{
		return false;
	}

	const float divider_ratio = (NTC_REFERENCE_VOLTAGE_V / voltage_v) - 1.0f;
	if (divider_ratio <= 0.0f)
	{
		return false;
	}

	const float thermistor_resistance = NTC_SERIES_RESISTOR_OHM / divider_ratio;
	if (thermistor_resistance <= 0.0f)
	{
		return false;
	}

	const float nominal_temperature_kelvin = NTC_NOMINAL_TEMPERATURE_C + SENSOR_KELVIN_OFFSET_C;
	const float steinhart = logf(thermistor_resistance / NTC_NOMINAL_RESISTANCE_OHM) / NTC_BETA_COEFFICIENT;
	const float temperature_kelvin = 1.0f / (steinhart + (1.0f / nominal_temperature_kelvin));

	if (!isfinite(temperature_kelvin))
	{
		return false;
	}

	*temperature_c = temperature_kelvin - SENSOR_KELVIN_OFFSET_C;
	return true;
}

static bool dd_temp_sensor_compute_sample_from_adc(float adc_value,
											   float *voltage_v,
											   float *temperature_c)
{
	if (voltage_v == NULL || temperature_c == NULL)
	{
		return false;
	}

	if (!dd_temp_sensor_adc_to_voltage(adc_value, voltage_v))
	{
		return false;
	}

	if (!dd_temp_sensor_voltage_to_celsius(*voltage_v, temperature_c))
	{
		return false;
	}

	return true;
}

static bool dd_temp_sensor_adc_to_celsius(float adc_value, float *temperature_c)
{
	float voltage_v = 0.0f;
	float sampled_temperature = 0.0f;

	if (!dd_temp_sensor_compute_sample_from_adc(adc_value,
											&voltage_v,
											&sampled_temperature))
	{
		return false;
	}

	*temperature_c = dd_temp_sensor_clamp(sampled_temperature,
										  SENSOR_TEMPERATURE_MIN_C,
										  SENSOR_TEMPERATURE_MAX_C);
	return true;
}

static void dd_temp_sensor_reset_runtime_state(dd_temp_sensor_t *sensor)
{
	sensor->temperature_c = 0.0f;
	sensor->last_voltage_v = 0.0f;
	sensor->raw_temperature_c = 0.0f;
	sensor->salt_pepper_temperature_c = 0.0f;
	sensor->raw_adc_value = 0U;
	sensor->salt_pepper_adc_value = 0U;
	sensor->last_adc_value = 0U;
	sensor->is_initialized = 0U;
	sensor->status = DD_TEMP_SENSOR_ERROR;
}

static void dd_temp_sensor_capture_stage_adc_values(dd_temp_sensor_t *sensor,
											 uint16_t raw_adc,
											 float salt_pepper_adc,
											 float weighted_adc)
{
	sensor->raw_adc_value = raw_adc;
	sensor->salt_pepper_adc_value = dd_temp_sensor_adc_to_u16(salt_pepper_adc);
	sensor->last_adc_value = dd_temp_sensor_adc_to_u16(weighted_adc);
}

static void dd_temp_sensor_update_stage_temperatures(dd_temp_sensor_t *sensor,
											 float raw_adc,
											 float salt_pepper_adc)
{
	float raw_temperature = 0.0f;
	float salt_pepper_temperature = 0.0f;

	if (dd_temp_sensor_adc_to_celsius(raw_adc, &raw_temperature))
	{
		sensor->raw_temperature_c = raw_temperature;
	}

	if (dd_temp_sensor_adc_to_celsius(salt_pepper_adc, &salt_pepper_temperature))
	{
		sensor->salt_pepper_temperature_c = salt_pepper_temperature;
	}
}

void dd_temp_sensor_init(dd_temp_sensor_t *sensor, uint8_t pin)
{
	if (sensor == NULL)
	{
		return;
	}

	sensor->pin = pin;
	dd_temp_sensor_reset_runtime_state(sensor);

	pinMode(sensor->pin, INPUT);
	filtru_sare_si_piper_init(&sensor->salt_pepper_filter);
	filtru_medie_ponderata_init(&sensor->weighted_filter);

	sensor->is_initialized = 1U;
	sensor->status = DD_TEMP_SENSOR_OK;
}

bool dd_temp_sensor_read_celsius(dd_temp_sensor_t *sensor, float *temperature_c)
{
	if (sensor == NULL || sensor->is_initialized == 0U)
	{
		if (sensor != NULL)
		{
			sensor->status = DD_TEMP_SENSOR_ERROR;
		}

		return false;
	}

	const uint16_t raw_adc = (uint16_t)analogRead(sensor->pin);
	const float raw_adc_float = (float)raw_adc;
	const float salt_pepper_adc = filtru_sare_si_piper_apply(&sensor->salt_pepper_filter,
												 raw_adc_float);
	const float weighted_adc = filtru_medie_ponderata_apply(&sensor->weighted_filter,
											  salt_pepper_adc);

	dd_temp_sensor_capture_stage_adc_values(sensor,
										 raw_adc,
										 salt_pepper_adc,
										 weighted_adc);

	dd_temp_sensor_update_stage_temperatures(sensor,
										raw_adc_float,
										salt_pepper_adc);

	float weighted_voltage = 0.0f;
	float weighted_temperature = 0.0f;
	if (!dd_temp_sensor_compute_sample_from_adc(weighted_adc,
											&weighted_voltage,
											&weighted_temperature))
	{
		sensor->status = DD_TEMP_SENSOR_ERROR;
		return false;
	}

	sensor->last_voltage_v = weighted_voltage;
	sensor->temperature_c = dd_temp_sensor_clamp(weighted_temperature,
											 SENSOR_TEMPERATURE_MIN_C,
											 SENSOR_TEMPERATURE_MAX_C);
	sensor->status = DD_TEMP_SENSOR_OK;

	if (temperature_c != NULL)
	{
		*temperature_c = sensor->temperature_c;
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
