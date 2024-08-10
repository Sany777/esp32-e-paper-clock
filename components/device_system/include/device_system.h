#ifndef DEVICE_SYSTEM_H
#define DEVICE_SYSTEM_H


#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

enum BasicConst{
    WEEK_DAYS_NUM   = 7,
    MAX_STR_LEN     = 32,
    API_LEN         = 32,
    NOT_ALLOVED_SOUND_TIME = 6*60,
};

enum Bits{
    BIT_SOUNDS_ALLOW        = (1<<0),
    BIT_AUTO_OFFSET         = (1<<1),
    BIT_STA_DISABLE         = (1<<2),
    BIT_STA_CONF_OK         = (1<<3),
    BIT_SNTP_OK             = (1<<4),
    BIT_ERR_SSID_NO_FOUND   = (1<<5),
    BIT_TRY_CONNECT         = (1<<6),
    BIT_IS_AP_MODE          = (1<<7),
    BIT_IS_AP_CONNECTION    = (1<<8),
    BIT_IS_STA_CONNECTION   = (1<<9),
    BIT_IS_WIFI_INIT        = (1<<10),
    BIT_IS_TIME             = (1<<11),
    BIT_SERVER_RUN          = (1<<12),
    BIT_IS_AP_CLIENT        = (1<<13),
    BIT_IS_PROCCESS         = (1<<14),
    STORED_FLAGS = (BIT_SOUNDS_ALLOW|BIT_AUTO_OFFSET|BIT_STA_DISABLE),
    NUMBER_STORED_FLAGS = 3
};

typedef struct {
    char ssid[MAX_STR_LEN];
    char pwd[MAX_STR_LEN];
    char city_name[MAX_STR_LEN];
    char api_key[API_LEN+1];
    unsigned flags;
    int time_offset;
    unsigned schema[WEEK_DAYS_NUM];
    unsigned *notification;
} clock_data_t;



int device_set_pwd(const char *str);
int device_set_ssid(const char *str);
int device_set_city(const char *str);
int device_set_key(const char *str);
int device_commit_changes();
unsigned device_get_state();
unsigned device_set_state(unsigned bits);
unsigned device_clear_state(unsigned bits);
unsigned device_wait_bits(unsigned bits);
void device_set_notify_data(unsigned *schema, unsigned *notif_data);
bool is_signale(int cur_min, int cur_day);
unsigned *device_get_schema();
unsigned * device_get_notif();
char *device_get_ssid();
char *device_get_pwd();
char *device_get_api_key();
char *device_get_city_name();
void device_sleep(const unsigned sleep_time_ms);
unsigned get_notif_num(unsigned *schema);
void device_system_init();


#define get_notif_size(schema) (get_notif_num(schema)*sizeof(unsigned))









#ifdef __cplusplus
}
#endif



#endif