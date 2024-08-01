#include "clock_system.h"

#include "clock_memory.h"
#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "semaphore.h"
#include <clock_macro.h>
#include "string.h"
#include "wifi_service.h"



static bool changes_main_data, changes_notify_data;
static clock_data_t main_data;
static SemaphoreHandle_t recursive_mux;
static EventGroupHandle_t clock_event_group;
static const char *MAIN_DATA_NAME = "main_data";
static const char *NOTIFY_DATA_NAME = "notify_data";



static int read_data();


unsigned get_notif_num(unsigned *schema)
{
    unsigned res = 0;
    for(int i=0; i<WEEK_DAYS_NUM; ++i){
        res += schema[i];
    }
    return res;
}




int device_set_pwd(const char *str)
{
    const int len = strnlen(str, API_LEN);
    if(len == MAX_STR_LEN || len == 0)return ESP_ERR_INVALID_SIZE;
    memcpy(main_data.pwd, str, len);
    main_data.pwd[len] = 0;
    changes_main_data = true;
    return ESP_OK;
}

int device_set_ssid(const char *str)
{
    const int len = strnlen(str, API_LEN);
    if(len == MAX_STR_LEN || len == 0)return ESP_ERR_INVALID_SIZE;
    memcpy(main_data.ssid, str, len);
    main_data.ssid[len] = 0;
    changes_main_data = true;
    return ESP_OK;
}

int device_set_city(const char *str)
{
    const int len = strnlen(str, API_LEN);
    if(len == MAX_STR_LEN || len == 0)return ESP_ERR_INVALID_SIZE;
    memcpy(main_data.city_name, str, len);
    main_data.city_name[len] = 0;
    changes_main_data = true;
    return ESP_OK;
}

int device_set_key(const char *str)
{
    if(strnlen(str, API_LEN) != API_LEN)return ESP_ERR_INVALID_SIZE;
    memcpy(main_data.api_key, str, API_LEN);
    changes_main_data = true;
    main_data.api_key[API_LEN] = 0;
    return ESP_OK;
}

void device_set_notify_data(unsigned *schema, unsigned *notif_data)
{
    if(main_data.notification){
        free(main_data.notification);
        main_data.notification = NULL;
    }
    main_data.notification = notif_data;
    memcpy(main_data.schema, schema, sizeof(main_data.schema));
    changes_notify_data = true;
    changes_main_data = true;
}


int device_commit_changes()
{
    if(changes_main_data){
        CHECK_AND_RET_ERR(write_flash(MAIN_DATA_NAME, (uint8_t *)&main_data, sizeof(main_data)));
        changes_main_data = false;
    }
    if(changes_notify_data){
        CHECK_AND_RET_ERR(write_flash(NOTIFY_DATA_NAME, (uint8_t *)main_data.notification, get_notif_size(main_data.schema)));
        changes_notify_data = false;
    }
    return ESP_OK;
}

unsigned device_get_state()
{
    return xEventGroupGetBits(clock_event_group);
} 

unsigned device_set_state(unsigned bits)
{
    if(bits&STORED_FLAGS){
        main_data.flags |= bits;
        changes_main_data = true;
    }
    return xEventGroupSetBits(clock_event_group, (EventBits_t) (bits));
}

unsigned device_clear_state(unsigned bits)
{
    if(bits&STORED_FLAGS){
        main_data.flags &= ~bits;
        changes_main_data = true;
    }
    return xEventGroupClearBits(clock_event_group, (EventBits_t) (bits));
}

unsigned device_wait_bits(unsigned bits)
{
    return xEventGroupWaitBits(clock_event_group,(EventBits_t) (bits),pdFALSE,pdFALSE,10000/portTICK_PERIOD_MS);
}

void device_system_init()
{
    clock_event_group = xEventGroupCreate();
    wifi_init();
    read_data();


}

unsigned *device_get_schema()
{
    return main_data.schema;
}

unsigned * device_get_notif()
{
    return main_data.notification;
}

char *device_get_ssid()
{
    return main_data.ssid;
}
char *device_get_pwd()
{
    return main_data.pwd;
}
char *device_get_api_key()
{
    return main_data.api_key;
}
char *device_get_city_name()
{
    return main_data.city_name;
}


static int read_data()
{
    CHECK_AND_RET_ERR(read_flash(MAIN_DATA_NAME, (unsigned char *)&main_data, sizeof(main_data)));
    const unsigned notif_data_byte_num = get_notif_size(main_data.schema);
    if(notif_data_byte_num){
        main_data.notification = (unsigned*)malloc(notif_data_byte_num);
        CHECK_AND_RET_ERR(read_flash(NOTIFY_DATA_NAME, (unsigned char *)main_data.notification, notif_data_byte_num));
    }
    return ESP_OK;
}


bool is_signale(int cur_min, int cur_day)
{
    const unsigned notif_num = main_data.schema[cur_day];
    unsigned *notif_data = main_data.notification;
    if( notif_num && notif_data
            && cur_min > NOT_ALLOVED_SOUND_TIME 
            && main_data.flags&BIT_SOUNDS_ALLOW ){
        for(int i=0; i<cur_day-1; ++i){
            notif_data += main_data.schema[i];
        }
        for(int i=0; i<notif_num; ++i){
            if(notif_data[i] == cur_min){
                return true;
            }
        }
    }
    return false;
}
