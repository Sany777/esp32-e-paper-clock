#include "clock_module.h"

#include <time.h>
#include <sys/time.h>
#include "device_system.h"
#include "string.h"
#include "esp_sntp.h"
#include "wifi_service.h"
#include "esp_err.h"

#include "additional_functions.h"

#define INTERVAL_10_HOUR   (1000*60*60*10)
#define INTERVAL_1_MIN      (1000*60*1)


struct tm* get_time_tm(void)
{
    static struct tm cur, *t;
    time_t time_now;
    time(&time_now);
    t = localtime(&time_now);
    memcpy(&cur, t, sizeof(struct tm));
    return &cur;
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
    settimeofday(tv, NULL);
    setenv("TZ", "EET2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();
    // first call
    if(esp_sntp_get_sync_interval() < INTERVAL_10_HOUR){
        esp_sntp_set_sync_interval(INTERVAL_10_HOUR);
        device_set_state(BIT_IS_TIME|BIT_SNTP_OK); 
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
int snprintf_time(char *strftime_buf, int buf_size, const char *format)
{
    if(!(device_get_state()&BIT_SNTP_OK)){
        init_sntp();
        if(! (device_wait_bits(BIT_IS_TIME|BIT_SNTP_OK)&BIT_IS_TIME ))return ESP_FAIL;
    }
    struct tm *timeinfo = get_time_tm();
    if(timeinfo->tm_year < (2016 - 1900))return ESP_FAIL;
    strftime(strftime_buf, buf_size, format, timeinfo);
    return ESP_OK;
}


void set_system_time(long long sec)
{
    sntp_set_system_time(sec, 0);
}