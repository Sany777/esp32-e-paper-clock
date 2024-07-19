#ifndef PERIODIC_TASK
#define PERIODIC_TASK



typedef void(*periodic_func_t)(void);

void stop_periodic_task(periodic_func_t func);
void new_periodic_task(periodic_func_t func, int delay_ms, int count);
int init_timer(void);
void stop_timer(void);
void tasks_run();


#endif



