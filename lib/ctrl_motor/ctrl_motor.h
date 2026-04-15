#ifndef ctrl_motor_h
#define ctrl_motor_h

#include <stdbool.h>
#include <stdint.h>

#include "dd_motor.h"

typedef enum
{
    CTRL_MOTOR_LIMIT_NONE = 0,
    CTRL_MOTOR_LIMIT_MAX,
    CTRL_MOTOR_LIMIT_MIN
} ctrl_motor_limit_event_t;

typedef struct
{
    int8_t target_power_percent;
    int8_t applied_power_percent;
    dd_motor_state_t motor_state;
    ctrl_motor_limit_event_t limit_event;
} ctrl_motor_status_t;

void ctrl_motor_init(void);
void ctrl_motor_handle_serial_char(char character);
void ctrl_motor_handle_keypad_char(char key);
void ctrl_motor_flush_serial_if_idle(void);
void ctrl_motor_control_step(void);
void ctrl_motor_report_status_if_due(void);
void ctrl_motor_get_status(ctrl_motor_status_t *status);
void ctrl_motor_print_command_help(void);

#endif // ctrl_motor_h
