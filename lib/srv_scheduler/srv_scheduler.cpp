#include "srv_scheduler.h"


void srv_scheduler_init(void)
{

    timer_init_ISR_1KHz(TIMER_DEFAULT);

}


void srv_scheduler_run(void)
{
    static uint32_t first_task_time = BUTTON_LED_TASK_OFFSET_MS;
    static uint32_t second_task_time = RED_LED_TASK_OFFSET_MS;
    static uint32_t third_task_time = INC_DEC_LED_TASK_OFFSET_MS;

    if (!(--first_task_time))
    {
        ctrl_button_led_task(NULL);
        first_task_time += BUTTON_LED_TASK_RECURRANCE_MS; // Schedule next run
    }

    if(!(--second_task_time)) 
    {
        ctrl_blink_led_task(NULL);
        second_task_time += RED_LED_TASK_RECURRANCE_MS; // Schedule next run
    }

    if(!(--third_task_time)) 
    {
        ctrl_inc_dec_led_task(NULL);
        third_task_time += INC_DEC_LED_TASK_RECURRANCE_MS; // Schedule next run
    }

    
    
}

void timer_handle_interrupts(int timer){
   srv_scheduler_run();
}