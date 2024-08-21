#include "clock_module.h"

#include <time.h>
#include "device_system.h"
#include "string.h"
#include "esp_sntp.h"
#include "wifi_service.h"
#include "esp_err.h"

#include "additional_functions.h"

#define INTERVAL_10_HOUR   (1000*60*60*10)
#define INTERVAL_1_MIN      (1000*60*1)

int get_time_in_min(struct tm* tinfo)
{
    return tinfo->tm_hour*60 + tinfo->tm_min;
}

struct tm* get_time_tm(void)
{
    time_t time_now;
    time(&time_now);
    return localtime(&time_now);
}


void set_time_ms(long long time_ms)
{
    struct timeval tv;
    tv.tv_sec = time_ms/1000;
    tv.tv_usec = time_ms%1000;
    settimeofday(&tv, NULL);
    device_set_state(BIT_IS_TIME);
}


static void set_time_cb(struct timeval *tv)
{
    unsigned bits = device_set_state(BIT_IS_TIME|BIT_SNTP_OK); 
    if(bits & BIT_OFFSET_ENABLE){
        tv->tv_sec += 60 * 60 * device_get_offset();
        settimeofday(tv, NULL);
    } else {
        setenv("TZ", "EET2EEST,M3.5.0/3,M10.5.0/4", 1);
        tzset();
        settimeofday(tv, NULL);
    }

    device_set_state(BIT_NEW_DATA);
    // first call
    if(esp_sntp_get_sync_interval() < INTERVAL_10_HOUR){
        esp_sntp_set_sync_interval(INTERVAL_10_HOUR);
    }
}


void init_sntp()
{
    if(esp_sntp_enabled()){
        esp_sntp_restart();
    } else {
        esp_sntp_set_time_sync_notification_cb(set_time_cb);
        esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_setservername(1, "time.windows.com");
        sntp_servermode_dhcp(0);
        esp_sntp_set_sync_interval(INTERVAL_1_MIN);
        esp_sntp_init();
    }
}


void stop_sntp()
{
    esp_sntp_stop();
    device_clear_state(BIT_SNTP_OK);
}

// format :
// %a: Аббревіатура дня тижня (Mon, Tue, ...)
// %A: Повна назва дня тижня (Monday, Tuesday, ...)
// %b: Аббревіатура місяця (Jan, Feb, ...)
// %B: Повна назва місяця (January, February, ...)
// %c: Дата і час (Sat Aug 23 14:55:02 2023)
// %d: День місяця (01 до 31)
// %H: Години в 24-годинному форматі (00 до 23)
// %I: Години в 12-годинному форматі (01 до 12)
// %j: День року (001 до 366)
// %m: Місяць (01 до 12)
// %M: Хвилини (00 до 59)
// %p: AM або PM
// %S: Секунди (00 до 60)
// %U: Номер тижня в році (неділя перший день тижня)
// %w: День тижня (неділя = 0, понеділок = 1, ...)
// %W: Номер тижня в році (понеділок перший день тижня)
// %x: Дата (08/23/23)
// %X: Час (14:55:02)
// %y: Останні дві цифри року (00 до 99)
// %Y: Повний рік (2023)
// %Z: Часовий пояс (UTC, GMT, ...)
const char* snprintf_time(const char *format)
{
    const char *err_res = "Err";
    static char text_buf[100];
    unsigned bits = device_get_state();
    if(!(bits&BIT_IS_TIME)){
        if( !( bits&BIT_IS_STA_CONNECTION) )
            return err_res;
        init_sntp();
        bits = device_wait_bits(BIT_IS_TIME);
        if(! (bits&BIT_IS_TIME ))
            return err_res;
    } 
    struct tm *timeinfo = get_time_tm();
    strftime(text_buf, sizeof(text_buf), format, timeinfo);
    return text_buf;
}


void set_system_time(long long sec)
{
    device_set_state(BIT_IS_TIME);
    sntp_set_system_time(sec, 0);
}