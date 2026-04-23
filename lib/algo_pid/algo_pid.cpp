#include "algo_pid.h"

void pid_init(pid_context_t *pid, float kp, float ki, float kd, float out_min, float out_max) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0;
    pid->prev_error = 0.0;
    pid->output_min = out_min;
    pid->output_max = out_max;
}

float pid_compute(pid_context_t *pid, float setpoint, float measured_value, float dt_seconds) {
    float error = setpoint - measured_value;
    
    // Proportional
    float p_term = pid->kp * error;
    
    // Integral (cu protectie Anti-Windup)
    pid->integral += error * dt_seconds;
    float i_term = pid->ki * pid->integral;
    
    // Derivativ
    float derivative = (error - pid->prev_error) / dt_seconds;
    float d_term = pid->kd * derivative;
    
    // Calcul output total
    float output = p_term + i_term + d_term;
    
    // Limitare output (Saturare 0 - 255 pentru PWM)
    if (output > pid->output_max) {
        output = pid->output_max;
        // Anti-windup: scadem integrala daca am atins limita de sus
        pid->integral -= error * dt_seconds; 
    } else if (output < pid->output_min) {
        output = pid->output_min;
        // Anti-windup: scadem integrala daca am atins limita de jos
        pid->integral -= error * dt_seconds;
    }
    
    pid->prev_error = error;
    return output;
}