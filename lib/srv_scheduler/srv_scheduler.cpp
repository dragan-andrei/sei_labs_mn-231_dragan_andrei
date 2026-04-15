#include "srv_scheduler.h"

#include <Arduino_FreeRTOS.h>

#include "ctrl_stdio.h"


void srv_scheduler_init(void)
{
    ctrl_stdio_serial_init();
    ctrl_stdio_lcd_init();
}


void srv_scheduler_run(void)
{

}
