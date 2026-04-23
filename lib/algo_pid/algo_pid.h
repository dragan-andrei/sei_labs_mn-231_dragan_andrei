#ifndef ALGO_PID_H
#define ALGO_PID_H

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
} pid_context_t;

void pid_init(pid_context_t *pid, float kp, float ki, float kd, float out_min, float out_max);
float pid_compute(pid_context_t *pid, float setpoint, float measured_value, float dt_seconds);

#endif