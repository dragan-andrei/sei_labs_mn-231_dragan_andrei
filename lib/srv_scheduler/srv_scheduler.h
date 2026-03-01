#ifndef SRV_SCHEDULER_H
#define SRV_SCHEDULER_H

#include "ctrl_tasks.h"
#include "configs.h"

void srv_scheduler_init(void);
void srv_scheduler_run(void);
void srv_scheduler_freertos_init(void);

#endif // SRV_SCHEDULER_H