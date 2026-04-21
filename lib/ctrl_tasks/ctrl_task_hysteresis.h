#ifndef CTRL_TASK_HYSTERESIS_H
#define CTRL_TASK_HYSTERESIS_H

#include <Arduino.h>

void ctrl_hysteresis_task_init(void);
void ctrl_setpoint_task(void *params);
void ctrl_hysteresis_control_task(void *params);

#endif // CTRL_TASK_HYSTERESIS_H
