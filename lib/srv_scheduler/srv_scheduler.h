#ifndef SRV_SCHEDULER_H
#define SRV_SCHEDULER_H

#include "ctrl_tasks.h"
#include "configs.h"
#include "timer-api.h"

void srv_scheduler_init(void);
void srv_scheduler_run(void);



#endif // SRV_SCHEDULER_H