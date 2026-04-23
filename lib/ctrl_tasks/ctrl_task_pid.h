#ifndef CTRL_TASK_PID_H
#define CTRL_TASK_PID_H

#ifdef __cplusplus
extern "C" {
#endif

// Init task-ul de PID
void ctrl_task_pid_init(void);

// Loop-ul OS de control (FreeRTOS)
void ctrl_pid_control_loop(void *params);

#ifdef __cplusplus
}
#endif

#endif // CTRL_TASK_PID_H