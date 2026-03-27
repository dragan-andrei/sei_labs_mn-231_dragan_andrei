#ifndef CTRL_TASK_SENSOR_H
#define CTRL_TASK_SENSOR_H

#include <Arduino.h>

void ctrl_sensor_task_init(void);
void ctrl_sensor_acquisition_task(void *params);
void ctrl_sensor_report_task(void *params);

#endif // CTRL_TASK_SENSOR_H
