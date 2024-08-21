#ifndef CLOCK_MODULE_H
#define CLOCK_MODULE_H


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>


struct tm* get_time_tm(void);
int get_time_in_min(struct tm* tinfo);
void init_sntp();
void stop_sntp();

const char* snprintf_time(const char *format);
void set_system_time(long long sec);
void set_time_ms(long long time_ms);

#ifdef __cplusplus
}
#endif





















#endif