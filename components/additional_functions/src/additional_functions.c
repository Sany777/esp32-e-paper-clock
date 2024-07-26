#include "additional_functions.h"

#include "esp_timer.h"
#include "esp_task.h"
#include "portmacro.h"
#include "esp_sleep.h"
#include "driver/gpio.h"

#define MAX_TASKS_NUM 10


typedef struct {
    periodic_func_t func;
    int delay_init;
    int count;
    int delay;
}periodic_event_t;


esp_timer_handle_t periodic_timer = NULL;
periodic_event_t periodic_tasks[MAX_TASKS_NUM];
portMUX_TYPE periodic_timers_s = portMUX_INITIALIZER_UNLOCKED;


static void periodic_timer_cb(void* arg);


void stop_periodic_task(periodic_func_t func)
{
    for(int i=0; i<MAX_TASKS_NUM; i++){
        if(periodic_tasks[i].func == func){
            periodic_tasks[i].count = 0;
            return;
        }
    }
}

void new_periodic_task(periodic_func_t func,
                            int delay_ms, 
                            int count)
{
    taskENTER_CRITICAL(&periodic_timers_s);

    periodic_event_t *item = NULL, *empty = NULL;
    for(int i=0; i<MAX_TASKS_NUM; i++){
        if(periodic_tasks[i].func == func){
            item = &periodic_tasks[i];
            break;
        } else if(empty == NULL && periodic_tasks[i].count == 0){
            empty = &periodic_tasks[i];
        }
    }

    if(item || empty){
        if(!item){
            item = empty;
            item->func = func;
        }

        item->delay = delay_ms;
        item->delay_init = delay_ms;
        item->count = count;


        if(periodic_timer == NULL){
            init_timer();
        }
        if(!esp_timer_is_active(periodic_timer)){
            esp_timer_start_periodic(periodic_timer, 1000);
        }
    }
    taskEXIT_CRITICAL(&periodic_timers_s);
}

void tasks_run()
{
    for(int i=0; i<MAX_TASKS_NUM; ++i){
        if(periodic_tasks[i].delay == 0 && periodic_tasks[i].count != 0){
            if(periodic_tasks[i].count > 0)--periodic_tasks[i].count;
            if(periodic_tasks[i].count != 0){
                periodic_tasks[i].delay = periodic_tasks[i].delay_init;
            }
            periodic_tasks[i].func();
        }
    }
}

static void periodic_timer_cb(void* arg)
{
    for(int i=0; i<MAX_TASKS_NUM; ++i){
        if(periodic_tasks[i].count != 0 && periodic_tasks[i].delay != 0){
            --periodic_tasks[i].delay;
            if(periodic_tasks[i].delay == 0){
                if(periodic_tasks[i].count > 0)--periodic_tasks[i].count;
                if(periodic_tasks[i].count != 0){
                    periodic_tasks[i].delay = periodic_tasks[i].delay_init;
                }
                periodic_tasks[i].func();
            }
        }
    }
}

int init_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_cb,
        .arg = NULL,
        .name = "tasks_timer"
    };
    return esp_timer_create(&periodic_timer_args, &periodic_timer);
}

void stop_timer(void)
{
    esp_timer_stop(periodic_timer);
}


void esp_sleep(const uint64_t sleep_time_ms)
{
    esp_sleep_enable_timer_wakeup(sleep_time_ms * 1000); 
    esp_deep_sleep_start();
}

int set_pin(int pin, uint8_t state)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);
    return gpio_set_level(pin, state);
}

