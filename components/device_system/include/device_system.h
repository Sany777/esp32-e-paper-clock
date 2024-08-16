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
    BIT_SOUNDS_DISABLE      = (1<<0),
    BIT_OFFSET_DISABLE      = (1<<1),
    BIT_STA_DISABLE         = (1<<2),
    BIT_STA_CONF_OK         = (1<<3),
    BIT_SNTP_OK             = (1<<4),
    BIT_OPENWEATHER_OK      = (1<<5),
    BIT_ERR_SSID_NO_FOUND   = (1<<6),
    BIT_STA_TRY_CONNECT     = (1<<7),
    BIT_IS_AP_MODE          = (1<<8),
    BIT_IS_AP_CONNECTION    = (1<<9),
    BIT_IS_STA_CONNECTION   = (1<<10),
    BIT_IS_WIFI_INIT        = (1<<11),
    BIT_IS_TIME             = (1<<12),
    BIT_SERVER_RUN          = (1<<13),
    BIT_IS_AP_CLIENT        = (1<<14),
    BIT_WAIT_PROCCESS         = (1<<15),

    BIT_START_SERVER        = (1<<16),
    BIT_START_OPENWEATHER   = (1<<17),
    BIT_START_SNTP          = (1<<18),
    BIT_START_MPU_DATA_UPDATE  = (1<<19),
    BIT_START_UPDATE_SCR    = (1<<20),
    BIT_NEW_DATA            = (1<<21),
    STORED_FLAGS = (BIT_SOUNDS_DISABLE|BIT_OFFSET_DISABLE|BIT_STA_DISABLE),
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


typedef struct {
    int cur_min;
    float temp_list[5];
    char desciption[20];
} service_data_t;

void device_gpio_init();
int device_get_joystick_btn();
int device_set_pin(int pin, unsigned state);

void set_bit_from_isr(unsigned bits);

#define GPIO_WAKEUP_PIN     (34)
#define AHT21_EN_PIN        (23)
#define SIG_OUT_PIN         (18) 
#define MPU6500_EN_PIN      (19)
#define I2C_MASTER_SCL_IO   (26)       
#define I2C_MASTER_SDA_IO   (25)        
#define EP_ON_PIN           (22)



enum CMD{
    BUT_RIGHT,
    BUT_PRESS,
    BUT_LEFT,
    NO_INP_DATA = -1,
};


enum PinoutInfo{
    EP_CS       = 4,
    EP_DC       = 16,
    EP_RST      = 17,
    EP_BUSY     = 5,
    EP_SDA      = 15,
    EP_SCL      = 14,
};

int device_set_pwd(const char *str);
int device_set_ssid(const char *str);
int device_set_city(const char *str);
int device_set_key(const char *str);
int device_commit_changes();
unsigned device_get_state();
unsigned device_set_state(unsigned bits);
unsigned device_clear_state(unsigned bits);
unsigned device_wait_bits_untile(unsigned bits, unsigned time_ms);
void device_set_notify_data(unsigned *schema, unsigned *notif_data);
bool is_signale(int cur_min, int cur_day);
unsigned *device_get_schema();
unsigned * device_get_notif();
char *device_get_ssid();
char *device_get_pwd();
char *device_get_api_key();
char *device_get_city_name();
int get_time_in_min();
unsigned get_notif_num(unsigned *schema);
void device_system_init();


#define device_wait_bits(bits) \
    device_wait_bits_untile(bits, 10000)
    
#define get_notif_size(schema) \
    (get_notif_num(schema)*sizeof(unsigned))



extern service_data_t service_data;





#ifdef __cplusplus
}
#endif



#endif