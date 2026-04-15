#ifndef CTRL_TASK_MOTOR_H
#define CTRL_TASK_MOTOR_H

void ctrl_motor_task_init(void);
void ctrl_motor_command_task(void *params);
void ctrl_motor_control_task(void *params);
void ctrl_motor_status_task(void *params);

#endif // CTRL_TASK_MOTOR_H
