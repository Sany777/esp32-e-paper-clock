#include "device_system.h"


#include "clock_module.h"

#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "semaphore.h"
#include "string.h"
#include "portmacro.h"
#include "esp_sleep.h"

#include "device_macro.h"
#include "wifi_service.h"
#include "device_memory.h"

#include "i2c_module.h"
#include "epaper_adapter.h"
#include "adc_reader.h"
#include "AHT21.h"
#include "MPU6500.h"
#include "periodic_task.h"
#include "sound_generator.h"

#include "freertos/FreeRTOS.h"
#include "MPU6500.h"
#include "epaper_adapter.h"

#include "esp_log.h"


static bool changes_main_data, changes_notify_data;
static settings_data_t main_data;
service_data_t service_data;
char network_buf[NET_BUF_LEN];

static EventGroupHandle_t clock_event_group;
static const char *MAIN_DATA_NAME = "main_data";
static const char *NOTIFY_DATA_NAME = "notify_data";

static int read_data();


void device_set_offset(int time_offset)
{
    if(main_data.flags & BIT_OFFSET_ENABLE){
        set_offset(time_offset - main_data.time_offset);
    }
    main_data.time_offset = time_offset;
    changes_main_data = true;
}

void device_set_loud(int loud)
{
    set_loud(loud);
    main_data.loud = loud;
    changes_main_data = true;
}

unsigned device_get_loud()
{
    return main_data.loud;
}


unsigned get_notif_num(unsigned *schema)
{
    unsigned res = 0;
    unsigned *end = schema + WEEK_DAYS_NUM;
    while(schema != end){
        res += *(schema++);
    }
    return res;
}

void device_set_pwd(const char *str)
{
    const int len = strnlen(str, MAX_STR_LEN);
    memcpy(main_data.pwd, str, len);
    main_data.pwd[len] = 0;
    changes_main_data = true;
}

void device_set_ssid(const char *str)
{
    const int len = strnlen(str, MAX_STR_LEN);
    memcpy(main_data.ssid, str, len);
    main_data.ssid[len] = 0;
    changes_main_data = true;
}

void device_set_city(const char *str)
{
    const int len = strnlen(str, MAX_STR_LEN);
    memcpy(main_data.city_name, str, len);
    main_data.city_name[len] = 0;
    changes_main_data = true;
}

void device_set_key(const char *str)
{
    if(strnlen(str, API_LEN+1) == API_LEN){
        memcpy(main_data.api_key, str, API_LEN);
        changes_main_data = true;
        main_data.api_key[API_LEN] = 0;
    }
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

unsigned  device_set_state(unsigned bits)
{
    if(bits&STORED_FLAGS){
        main_data.flags |= bits;
        changes_main_data = true;
    }
    return xEventGroupSetBits(clock_event_group, (EventBits_t) (bits));
}

unsigned  device_clear_state(unsigned bits)
{
    if(bits&STORED_FLAGS){
        main_data.flags &= ~bits;
        changes_main_data = true;
    }
    return xEventGroupClearBits(clock_event_group, (EventBits_t) (bits));
}

unsigned device_wait_bits_untile(unsigned bits, unsigned time_ticks)
{
    return xEventGroupWaitBits(clock_event_group,
                                (EventBits_t) (bits),
                                pdFALSE,
                                pdFALSE,
                                time_ticks);
}


unsigned  *device_get_schema()
{
    return main_data.schema;
}

unsigned *  device_get_notif()
{
    return main_data.notification;
}

char *  device_get_ssid()
{
    return main_data.ssid;
}
char *  device_get_pwd()
{
    return main_data.pwd;
}
char *  device_get_api_key()
{
    return main_data.api_key;
}
char *  device_get_city_name()
{
    return main_data.city_name;
}

int device_get_offset()
{
    return main_data.time_offset;
}


static int read_data()
{
    CHECK_AND_RET_ERR(read_flash(MAIN_DATA_NAME, (unsigned char *)&main_data, sizeof(main_data)));
    device_set_state(main_data.flags&STORED_FLAGS);
    set_loud(main_data.loud);
    const unsigned notif_data_byte_num = get_notif_size(main_data.schema);
    if(notif_data_byte_num){
        main_data.notification = (unsigned*)malloc(notif_data_byte_num);
        CHECK_AND_RET_ERR(read_flash(NOTIFY_DATA_NAME, (unsigned char *)main_data.notification, notif_data_byte_num));
    }
    return ESP_OK;
}


bool is_signale(int cur_min, int cur_day)
{
    cur_day = (cur_day+1) % 7;
    const unsigned notif_num = main_data.schema[cur_day];
    unsigned *notif_data = main_data.notification;
    if( notif_num && notif_data
            && cur_min > NOT_ALLOVED_SOUND_TIME 
            && !(main_data.flags&BIT_SOUNDS_DISABLE) ){
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



void device_system_init()
{
    clock_event_group = xEventGroupCreate();
    if(clock_event_group == NULL){
        esp_restart();
    }
    device_set_pin(EP_ON_PIN, 0);
    device_set_pin(AHT21_EN_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
    device_timer_start();
    I2C_init();
    device_set_pin(EP_ON_PIN, 1);
    device_set_pin(AHT21_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    read_data();
    wifi_init();
    adc_reader_init();
    mpu_init();
    AHT21_init();
    epaper_init();
    device_gpio_init();
}



void  set_bit_from_isr(unsigned bits)
{
    BaseType_t pxHigherPriorityTaskWoken;
    xEventGroupSetBitsFromISR(clock_event_group, (EventBits_t)bits, &pxHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );
}

void  clear_bit_from_isr(unsigned bits)
{
    xEventGroupClearBitsFromISR(clock_event_group, (EventBits_t)bits);
}
