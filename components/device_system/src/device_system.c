#include "device_system.h"

#include "stdlib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "semaphore.h"
#include "string.h"
#include "portmacro.h"
#include "esp_sleep.h"

#include "device_macro.h"
#include "wifi_service.h"
#include "device_system.h"
#include "device_memory.h"

#include "i2c_module.h"
#include "epaper_adapter.h"
#include "adc_reader.h"
#include "AHT21.h"
#include "MPU6500.h"
#include "periodic_task.h"


static bool changes_main_data, changes_notify_data;
static clock_data_t main_data;
service_data_t service_data;

static EventGroupHandle_t clock_event_group;
static const char *MAIN_DATA_NAME = "main_data";
static const char *NOTIFY_DATA_NAME = "notify_data";


void IRAM_ATTR clear_bit_from_isr(unsigned bits);
static int read_data();




unsigned get_notif_num(unsigned *schema)
{
    unsigned res = 0;
    unsigned *end = schema + WEEK_DAYS_NUM;
    while(schema != end){
        res += *(schema++);
    }
    return res;
}

int device_set_pwd(const char *str)
{
    device_clear_state(BIT_STA_CONF_OK);
    const int len = strnlen(str, MAX_STR_LEN);
    if(len >= MAX_STR_LEN){
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(main_data.pwd, str, len);
    main_data.pwd[len] = 0;
    changes_main_data = true;
    return ESP_OK;
}

int device_set_ssid(const char *str)
{
    device_clear_state(BIT_STA_CONF_OK);
    const int len = strnlen(str, MAX_STR_LEN);
    if(len == MAX_STR_LEN)return ESP_ERR_INVALID_SIZE;
    memcpy(main_data.ssid, str, len);
    main_data.ssid[len] = 0;
    changes_main_data = true;
    return ESP_OK;
}

int device_set_city(const char *str)
{
    const int len = strnlen(str, MAX_STR_LEN);
    if(len >= MAX_STR_LEN)return ESP_ERR_INVALID_SIZE;
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

unsigned IRAM_ATTR device_set_state(unsigned bits)
{
    if(bits&STORED_FLAGS){
        main_data.flags |= bits;
        changes_main_data = true;
    }
    return xEventGroupSetBits(clock_event_group, (EventBits_t) (bits));
}

unsigned IRAM_ATTR device_clear_state(unsigned bits)
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


unsigned IRAM_ATTR *device_get_schema()
{
    return main_data.schema;
}

unsigned * IRAM_ATTR device_get_notif()
{
    return main_data.notification;
}

char * IRAM_ATTR device_get_ssid()
{
    return main_data.ssid;
}
char * IRAM_ATTR device_get_pwd()
{
    return main_data.pwd;
}
char * IRAM_ATTR device_get_api_key()
{
    return main_data.api_key;
}
char * IRAM_ATTR device_get_city_name()
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
    device_timer_start();
    I2C_init();
    device_set_pin(EP_ON_PIN, 1);
    device_set_pin(AHT21_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    read_data();
    device_gpio_init();
    wifi_init();
    adc_reader_init();
    mpu_init();
    AHT21_init();
    epaper_init();
}




#include "sound_generator.h"
#include "device_system.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "MPU6500.h"
#include "epaper_adapter.h"

#include "portmacro.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "periodic_task.h"

// 34 - UP, 27 - left, 35 - right, 32 - , 33 -center,


static const int joystic_pin[] = {GPIO_NUM_35,GPIO_NUM_33,GPIO_NUM_27};
static const int BUT_NUM = sizeof(joystic_pin)/sizeof(joystic_pin[0]);


static void IRAM_ATTR send_sig_update_pos()
{
    clear_bit_from_isr(BIT_WAIT_MOVING);
}

void IRAM_ATTR set_bit_from_isr(unsigned bits)
{
    BaseType_t pxHigherPriorityTaskWoken;
    xEventGroupSetBitsFromISR(clock_event_group, (EventBits_t)bits, &pxHigherPriorityTaskWoken);
    portYIELD_FROM_ISR( pxHigherPriorityTaskWoken );
}

void IRAM_ATTR clear_bit_from_isr(unsigned bits)
{
    xEventGroupClearBitsFromISR(clock_event_group, (EventBits_t)bits);
}


void IRAM_ATTR gpio_isr_handler(void* arg)
{
    set_bit_from_isr(BIT_WAIT_MOVING);
    periodic_task_isr_create(send_sig_update_pos, 300, 1);
}

void setup_gpio_interrupt()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE, 
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_WAKEUP_PIN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_WAKEUP_PIN, gpio_isr_handler, NULL);
}

int IRAM_ATTR device_set_pin(int pin, unsigned state)
{
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_INPUT_OUTPUT);
    return gpio_set_level((gpio_num_t )pin, state);
}


void device_gpio_init()
{
    for(int i=0; i<BUT_NUM; ++i){
        gpio_set_direction(joystic_pin[i], GPIO_MODE_INPUT);
        gpio_pulldown_en(joystic_pin[i]);
    }
    setup_gpio_interrupt();
}

static void end_but_input()
{
    clear_bit_from_isr(BIT_BUT_INPUT);
    set_bit_from_isr(BIT_NEW_DATA);
}

int device_get_joystick_btn()
{
    int timeout = 10;
    for(int i=0; i<BUT_NUM; ++i){
        if(gpio_get_level(joystic_pin[i])){
            device_set_state(BIT_BUT_INPUT);
            do{
                timeout -= 1;
                vTaskDelay(100/portTICK_PERIOD_MS);
                periodic_task_isr_create(end_but_input, 5000, 1);
            }while(gpio_get_level(joystic_pin[i]) && timeout);
            return i;
        }
    }
    return NO_DATA;
}