#ifndef CLOCK_SYSTEM_H
#define CLOCK_SYSTEM_H


#define MAX_STR_LEN 32
#define API_LEN 32


#include "stdint.h"

#include "freertos/FreeRTOS.h"

enum _bits{
    BIT_IS_WIFI_INIT        = (1<<0),
    BIT_IS_STA_CONNECTION   = (1<<1),
    BIT_IS_TIME             = (1<<2),
    BIT_IS_AP_MODE          = (1<<3),
    BIT_IS_AP_CONNECTION    = (1<<4),
    BIT_ERR_SSID_NO_FOUND   = (1<<5),
    BIT_TRY_CONNECT         = (1<<6),
    BIT_STA_CONF_OK         = (1<<7),
    BIT_SNTP_OK             = (1<<8),
    BIT_AUTO_OFFSET         = (1<<9),
    BIT_SOUNDS_ALLOW        = (1<<10),
    BIT_STA_DISABLE         = (1<<10)
};
#define STORED_FLAGS (1UL|BIT_SOUNDS_ALLOW|BIT_AUTO_OFFSET|BIT_STA_DISABLE)


typedef struct{
    char ssid[MAX_STR_LEN];
    char pwd[MAX_STR_LEN];
    char city_name[MAX_STR_LEN];
    char api_key[API_LEN+1];
    int *notification;
    int time_offset;
    int notification_num;
}clock_data_t;

extern EventGroupHandle_t clock_event_group;

#define get_device_state() \
    (xEventGroupGetBits(clock_event_group))

#define set_device_state(_bits) \
    (xEventGroupSetBits(clock_event_group, (_bits)))

#define clear_device_state(_bits) \
    (xEventGroupClearBits(clock_event_group, (_bits)))

#define wait_bits(_bits) \
    (xEventGroupWaitBits(clock_event_group,(_bits),pdFALSE,pdFALSE,10000/portTICK_PERIOD_MS))




void clock_system_init();

#endif