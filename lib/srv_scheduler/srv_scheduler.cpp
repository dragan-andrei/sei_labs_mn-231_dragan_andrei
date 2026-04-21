#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>

#include "ctrl_stdio.h"


void srv_scheduler_init(void)
{
    ctrl_stdio_serial_init();
    ctrl_stdio_lcd_init();
    ctrl_sensor_task_init();

    xTaskCreate(ctrl_sensor_acquisition_task,
                "sensor_acq",
                SENSOR_ACQUISITION_TASK_STACK_SIZE,
                NULL,
                SENSOR_ACQUISITION_TASK_PRIORITY,
                NULL);

    xTaskCreate(ctrl_sensor_report_task,
                "sensor_rep",
                SENSOR_REPORT_TASK_STACK_SIZE,
                NULL,
                SENSOR_REPORT_TASK_PRIORITY,
                NULL);

    ctrl_hysteresis_task_init();

    xTaskCreate(ctrl_setpoint_task,
                "setpoint",
                SETPOINT_TASK_STACK_SIZE,
                NULL,
                SETPOINT_TASK_PRIORITY,
                NULL);

    xTaskCreate(ctrl_hysteresis_control_task,
                "hyst_ctrl",
                HYSTERESIS_TASK_STACK_SIZE,
                NULL,
                HYSTERESIS_TASK_PRIORITY,
                NULL);

}


void srv_scheduler_run(void)
{

}
