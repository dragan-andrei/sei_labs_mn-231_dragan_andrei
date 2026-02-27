#ifndef CTRL_TASKS_H
#define CTRL_TASKS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "ctrl_stdio.h"
#include "dd_led.h"
#include "dd_button.h"
#include "configs.h"

void ctrl_button_led_task_init(dd_button_t *button, dd_led_t *led);
void ctrl_button_led_task(void *params);

void ctrl_blink_led_task_init(dd_led_t *led);
void ctrl_blink_led_task(void *params);

void ctrl_inc_dec_led_task_init(dd_button_t *button_up, dd_button_t *button_down);
void ctrl_inc_dec_led_task(void *params);

void ctrl_idle_task_init(void);
void ctrl_idle_task(void *params);

#endif // CTRL_TASKS_H  