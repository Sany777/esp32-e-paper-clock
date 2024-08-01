#include "periodic_taks.h"

#include "esp_timer.h"
#include "esp_task.h"
#include "portmacro.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MAX_TASKS_NUM 10

static TaskHandle_t task_runner_handle;

typedef struct {
    void *func;
    int delay_init;
    int count;
    int delay;
    void* context;
    bool task_in_interrupt;
}periodic_task_data_t;

esp_timer_handle_t periodic_timer = NULL;
periodic_task_data_t periodic_tasks[MAX_TASKS_NUM] = {0};
portMUX_TYPE periodic_timers_s = portMUX_INITIALIZER_UNLOCKED;


static int new_periodic_task(void *func,
                            unsigned delay_ms, 
                            unsigned count,
                            void *context,
                            bool run_in_intrp);

static void task_runner(void *pvParameters);
static IRAM_ATTR void periodic_timer_cb(void* arg);
static int init_timer(void);


periodic_task_data_t* find_task(void* func)
{
    if(!func)return NULL;
    periodic_task_data_t *end = periodic_tasks+MAX_TASKS_NUM, *ptr = periodic_tasks;
    while(ptr < end){
        if(ptr->task_in_interrupt){
            if(ptr->func == func){
                return ptr;
            }
        } else if(ptr->func == func){
            return ptr;
        } 
        ++ptr;
    }
    return NULL;
}

void periodic_task_remove(void *func)
{
    periodic_task_data_t*to_delete = find_task(func);
    if(to_delete){
        to_delete->count = 0;
    }
}

static int new_periodic_task(void *func,
                            unsigned delay_ms, 
                            unsigned count,
                            void *context,
                            bool run_in_intrp)
{
    int res = ESP_FAIL;
    if(!run_in_intrp){
        if(task_runner_handle == NULL){
            xTaskCreate(task_runner, "Task runner", 4096, NULL, 8, &task_runner_handle);
            if(task_runner_handle == NULL) return ESP_FAIL;
        }
    }
    task_runner_stop();
    periodic_task_data_t *end = periodic_tasks+MAX_TASKS_NUM, *ptr = periodic_tasks;
    periodic_task_data_t *to_insert = find_task(func);
    while(to_insert == NULL && ptr < end){
        if(ptr->count == 0){
            to_insert = ptr;
            to_insert->func = func;
        } else {
            ++ptr;
        }
    }

    if(to_insert){
        to_insert->delay = delay_ms;
        to_insert->delay_init = delay_ms;
        to_insert->count = count;
        to_insert->context = context;
        to_insert->task_in_interrupt = run_in_intrp;
        res = ESP_OK;
    }
    task_runner_start();
    return res;
}

int periodic_task_create(void(*func)(void*),
                            unsigned delay_ms, 
                            unsigned count,
                            void *context)
{
    return new_periodic_task(func, delay_ms, count, context, false);
}

int periodic_task_in_isr_create(void(*func)(),
                            unsigned delay_ms, 
                            unsigned count)
{
    return new_periodic_task(func, delay_ms, count, NULL, true);
}



static void task_runner(void *pvParameters)
{
    uint32_t ulNotificationValue;
    const periodic_task_data_t *end = periodic_tasks+MAX_TASKS_NUM;
    periodic_task_data_t *ptr;
    periodic_func_t func;
    vTaskDelay(100);
    for(;;){
        if (xTaskNotifyWait(0x00, 0x00, &ulNotificationValue, portMAX_DELAY) == pdTRUE){
            ptr = periodic_tasks;
            while(ptr < end){
                if(! ptr->task_in_interrupt && ptr->delay == 0 && ptr->count != 0){
                    if(ptr->count > 0) --ptr->count;
                    if(ptr->count != 0){
                        ptr->delay = ptr->delay_init;
                    }
                    func = (periodic_func_t)ptr->func;
                    func(ptr->context);
                } 
                ++ptr;
            }
        }
    }
}

static  void IRAM_ATTR periodic_timer_cb(void*)
{
    BaseType_t xHigherPriorityTaskWoken;
    bool need_start_runer = false;
    const periodic_task_data_t *end = periodic_tasks+MAX_TASKS_NUM;
    periodic_task_data_t *ptr = periodic_tasks;
    periodic_func_in_isr_t func;
    while(ptr < end){
        if(ptr->delay > 0){
            --ptr->delay;
            if(ptr->delay == 0){
                if(ptr->task_in_interrupt){
                    if(ptr->count > 0) --ptr->count;
                    if(ptr->count != 0){
                        ptr->delay = ptr->delay_init;
                    }
                    func = (periodic_func_in_isr_t)ptr->func;
                    func();
                }else{
                    need_start_runer = true;
                }
            }
        }
        ++ptr;
    }
    if(task_runner_handle){
        vTaskNotifyGiveFromISR(task_runner_handle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

static int init_timer(void)
{
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_cb,
        .arg = NULL,
        .name = "tasks timer",
        .skip_unhandled_events = false
    };
    return esp_timer_create(&periodic_timer_args, &periodic_timer);
}

void task_runner_start(void)
{
    if(task_runner_handle){
        vTaskResume(task_runner_handle);
    }
    if(periodic_timer == NULL){
        init_timer();
    }
    if(periodic_timer && !esp_timer_is_active(periodic_timer)){
        esp_timer_start_periodic(periodic_timer, 1000);
    }
}

void task_runner_stop()
{
    if(task_runner_handle){
        vTaskSuspend(task_runner_handle);
    }
    if(periodic_timer && esp_timer_is_active(periodic_timer)){
        esp_timer_stop(periodic_timer);
    }
}

void task_runner_deinit()
{
    if(task_runner_handle){
        vTaskDelete(task_runner_handle);
        task_runner_handle = NULL;
    }
    if(periodic_timer){
        esp_timer_stop(periodic_timer);
        esp_timer_delete(periodic_timer);
        periodic_timer = NULL;
    }
}

