#ifndef PERIODIC_TAKS_H
#define PERIODIC_TAKS_H


#ifdef __cplusplus
extern "C" {
#endif


typedef void(*periodic_func_t)(void);

void stop_periodic_task(periodic_func_t func);
void new_periodic_task(periodic_func_t func, int delay_ms, int count);
int init_timer(void);
void stop_timer(void);
void tasks_run();







#ifdef __cplusplus
}
#endif

#endif



