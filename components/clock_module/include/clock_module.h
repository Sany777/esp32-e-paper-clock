#ifndef CLOCK_MODULE_H
#define CLOCK_MODULE_H


#ifdef __cplusplus
extern "C" {
#endif

struct tm* get_time_tm(void);
void start_sntp();
void stop_sntp();
// example format: "%H:%M:%S"...
int snprintf_time(char *strftime_buf, int buf_size, const char *format);
void set_system_time(long long sec);
void set_time_ms(long long time_ms);


#ifdef __cplusplus
}
#endif





















#endif