#ifndef NET_SERVICE_H
#define NET_SERVICE_H

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif


enum Bits{
    BIT_STA_CON         = (1<<0),
    BIT_TIME_OK         = (1<<1),
    BIT_AP_MODE         = (1<<2),
    BIT_AP_CON          = (1<<3),
    BIT_SSID_NO_FOUND   = (1<<4),
    BIT_TRY_CONNECT     = (1<<5),
    BIT_STA_CONF_OK     = (1<<6),
    BIT_SNTP_OK         = (1<<7)
};

bool get_time(char *strftime_buf, int buf_size);
void set_wifi_sta_config(char *ssid, char *pwd);
int connect_sta();
int connect_ap();
void wifi_stop();
void init_sntp();
void stop_sntp();

int init_wifi(void) ;

#ifdef __cplusplus
}
#endif




#endif