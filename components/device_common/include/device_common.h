#ifndef DEVICE_SYSTEM_H
#define DEVICE_SYSTEM_H


#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"
#include "time.h"

enum BasicConst{
    WEEK_DAYS_NUM           = 7,
    MAX_STR_LEN             = 32,
    API_LEN                 = 32,
    FORBIDDED_NOTIF_HOUR    = 6*60,
    DESCRIPTION_SIZE        = 20,
    FORECAST_LIST_SIZE      = 5,
    NET_BUF_LEN             = 5000,
};

enum Bits{
    BIT_NOTIF_DISABLE           = (1<<0),
    BIT_STA_DISABLE             = (1<<1),
    BIT_OFFSET_ENABLE           = (1<<2),
    BIT_FORECAST_OK            = (1<<3),
    BIT_SNTP_OK                 = (1<<4),
    BIT_ERR_SSID_NO_FOUND       = (1<<5),
    BIT_WAIT_MOVING             = (1<<6),
    BIT_IS_AP_CONNECTION        = (1<<7),
    BIT_IS_STA_CONNECTION       = (1<<8),
    BIT_IS_TIME                 = (1<<9),
    BIT_SERVER_RUN              = (1<<10),
    BIT_IS_AP_CLIENT            = (1<<11),
    BIT_WAIT_PROCCESS           = (1<<12),
    BIT_START_SERVER            = (1<<13),
    BIT_UPDATE_FORECAST_DATA   = (1<<14),
    BIT_INIT_SNTP               = (1<<15),
    BIT_START_MPU_DATA_UPDATE   = (1<<16),
    BIT_WAIT_BUT_INPUT          = (1<<17),
    BIT_NEW_DATA                = (1<<18),
    BIT_NEW_MIN                 = (1<<19),
    BIT_WAIT_SIGNALE            = (1<<20),
    STORED_FLAGS                = (BIT_NOTIF_DISABLE|BIT_OFFSET_ENABLE|BIT_STA_DISABLE),
};

typedef struct {
    char ssid[MAX_STR_LEN+1];
    char pwd[MAX_STR_LEN+1];
    char city_name[MAX_STR_LEN+1];
    char api_key[API_LEN+1];
    unsigned flags;
    unsigned loud;
    int time_offset;
    unsigned schema[WEEK_DAYS_NUM];
    unsigned *notification;
} settings_data_t;


typedef struct {
    char desciption[FORECAST_LIST_SIZE][DESCRIPTION_SIZE+1];
    int cur_sec;
    int update_data_time;
    int pop_list[FORECAST_LIST_SIZE];
    float temp_list[FORECAST_LIST_SIZE];
} service_data_t;

void device_gpio_init();
int device_get_joystick_btn();
int device_set_pin(int pin, unsigned state);
bool device_wait_moving_end(int timeout_ms);

#define PIN_WAKEUP          (34)
#define PIN_AHT21_EN        (23)
#define PIN_SIG_OUT         (18) 
#define PIN_MPU6500_EN      (19)
#define PIN_EPAPER_EN       (22)
#define I2C_MASTER_SCL_IO   (26)       
#define I2C_MASTER_SDA_IO   (25)        

enum CMD{
    BUT_RIGHT,
    BUT_PRESS,
    BUT_LEFT,
    NO_DATA = -1,
};

enum PinoutInfo{
    EP_CS       = 4,
    EP_DC       = 16,
    EP_RST      = 17,
    EP_BUSY     = 5,
    EP_SDA      = 15,
    EP_SCL      = 14,
};

int device_get_offset();
void device_set_pwd(const char *str);
void device_set_ssid(const char *str);
void device_set_city(const char *str);
void device_set_key(const char *str);
int device_commit_changes();
unsigned device_get_state();
unsigned device_wait_bits_untile(unsigned bits, unsigned time_ms);
void device_set_notify_data(unsigned *schema, unsigned *notif_data);
bool is_signale(struct tm *tm_info);
unsigned *device_get_schema();
unsigned * device_get_notif();
char *device_get_ssid();
char *device_get_pwd();
char *device_get_api_key();
char *device_get_city_name();
void device_set_offset(int time_offset);
void device_set_loud(int loud);
unsigned device_get_loud();
unsigned device_clear_state(unsigned bits);
void device_set_state_isr(unsigned bits);
void device_clear_state_isr(unsigned bits);
unsigned device_set_state(unsigned bits);
unsigned get_notif_num(unsigned *schema);
void device_init();


#define device_wait_bits(bits) \
    device_wait_bits_untile(bits, 10000/portTICK_PERIOD_MS)
    
#define get_notif_size(schema) \
    (get_notif_num(schema)*sizeof(unsigned))


extern service_data_t service_data;

extern char network_buf[];






#ifdef __cplusplus
}
#endif



#endif