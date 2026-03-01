#include "srv_scheduler.h"

void srv_scheduler_init(void)
{
    srv_scheduler_freertos_init();
    /* FreeRTOS scheduler takes over — this line is never reached */
}

void srv_scheduler_run(void)
{
    /* Not used with FreeRTOS — scheduler handles everything */
}