#ifndef CTRL_TASK_SENSOR_H
#define CTRL_TASK_SENSOR_H

#include <Arduino.h>
#include <stdint.h>

#include "dd_temp_sensor.h"

typedef struct
{
    float last_temperature_c;
    float raw_temperature_c;
    float salt_pepper_temperature_c;
    float weighted_temperature_c;
    uint16_t raw_adc_value;
    uint16_t salt_pepper_adc_value;
    uint16_t weighted_adc_value;
    uint8_t is_data_valid;
    uint8_t is_alert_active;
    uint32_t sample_count;
    dd_temp_sensor_status_t sensor_status;
} ctrl_sensor_signals_t;

void ctrl_sensor_task_init(void);
void ctrl_sensor_acquisition_task(void *params);
void ctrl_sensor_report_task(void *params);
void ctrl_sensor_get_snapshot(ctrl_sensor_signals_t *snapshot);

#endif // CTRL_TASK_SENSOR_H
