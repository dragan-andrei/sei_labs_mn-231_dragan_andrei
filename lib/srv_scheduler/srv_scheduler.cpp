#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>
#include <task.h>

#include "configs.h"
#include "ctrl_stdio.h"
#include "ctrl_task_pid.h"

void srv_scheduler_init(void) {
    // 1. Inițializăm portul Serial pentru printf (Stdio redirection)
    ctrl_stdio_serial_init();

    // 2. Inițializăm contextul de drivere pentru Controller-ul Principal (PID)
    ctrl_task_pid_init();

    // 3. Creăm Task-ul responsabil cu PID Loop și I/O
    xTaskCreate(
        (TaskFunction_t)ctrl_pid_control_loop,
        "PID_Control",
        OS_PID_TASK_STACK_SIZE,
        NULL,
        OS_PID_TASK_PRIORITY,
        NULL
    );

    // * Scheduler-ul RTOS din library va porni automat după apelarea main()-ului în platforma Arduino/AVR.
}