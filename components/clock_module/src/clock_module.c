#include "clock_module.h"

#include <time.h>
#include <sys/time.h>
#include "device_system.h"
#include "string.h"
#include "esp_sntp.h"
#include "wifi_service.h"

#include "additional_functions.h"

struct tm* get_time_tm(void)
{
    static struct tm cur, *t;
    time_t time_now;
    time(&time_now);
    t = localtime(&time_now);
    memcpy(&cur, t, sizeof(struct tm));
    // fixDay(&cur);
    // performTimeAction(&cur);
    return &cur;
}

void set_time_ms(long long time_ms)
{
    struct timeval tv;
    tv.tv_sec = time_ms/1000;
    tv.tv_usec = time_ms%1000;
    settimeofday(&tv, NULL);
    // device_set_state(BIT_IS_TIME);
}


static void set_time_cb(struct timeval *tv)
{
    const int INTERVAL_10_HOUR = 1000*60*60*10;
    settimeofday(tv, NULL);
    setenv("TZ", "EET2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();
    // if the first call
    if(esp_sntp_get_sync_interval() < INTERVAL_10_HOUR){
        esp_sntp_set_sync_interval(INTERVAL_10_HOUR);
        // xEventGroupSetBits(clock_event_group, BIT_IS_TIME|BIT_SNTP_OK); 
    }
}





void start_sntp()
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
        const int INTERVAL_1_MIN = 1000*60*1;
        esp_sntp_set_sync_interval(INTERVAL_1_MIN);
        esp_sntp_init();
    }
}


void stop_sntp()
{
    esp_sntp_stop();
    // if(clock_event_group){
    //     // device_clear_state(BIT_SNTP_OK);
    // }
}

// format "%H:%M:%S"...
int snprintf_time(char *strftime_buf, int buf_size, const char *format)
{
    // EventBits_t bits = device_get_state();
    // if(! (bits&BIT_IS_WIFI_INIT) ){
    //     CHECK_AND_RET_ERR(wifi_init());
    // } 
    // if( !(bits&BIT_IS_STA_CONNECTION)){
    //     CHECK_AND_RET_ERR(connect_sta());
    // }
    // if(!(bits&BIT_SNTP_OK)){
    //     start_sntp();
    //     // if(! (device_wait_bits(BIT_SNTP_OK) &BIT_SNTP_OK))
    //     //     return ESP_FAIL;
    // }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, buf_size, format, &timeinfo);
    return timeinfo.tm_year > (2016 - 1900) ? ESP_OK : ESP_FAIL;
}

void set_system_time(long long sec)
{
    sntp_set_system_time(sec, 0);
}