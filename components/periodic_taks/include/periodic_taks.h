#ifndef PERIODIC_TAKS_H
#define PERIODIC_TAKS_H


#ifdef __cplusplus
extern "C" {
#endif


typedef void(*periodic_func_in_isr_t)();



void periodic_task_remove(periodic_func_in_isr_t func);
int periodic_task_isr_create(periodic_func_in_isr_t func,
                            unsigned delay_ms, 
                            unsigned count);
void task_runner_start(void);
void task_runner_stop(void);
void task_runner_deinit(void);






#ifdef __cplusplus
}
#endif

#endif



