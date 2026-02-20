#include "srv_scheduler.h"


void srv_scheduler_init(void){

    timer_init_ISR_1KHz();

}

void srv_scheduler_run(void){
    static uint32_t first_time_task
}

void timer_handle_interrupts(void){
   srv_scheduler_run();
}