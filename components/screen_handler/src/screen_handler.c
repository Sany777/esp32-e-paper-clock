#include "screen_handler.h"

#include "periodic_task.h"
#include "device_system.h"
#include "device_system.h"
#include "MPU6500.h"
#include "clock_http_client.h"
#include "sound_generator.h"
#include "AHT21.h"
#include "adc_reader.h"

#include "esp_sleep.h"
#include "wifi_service.h"

#include "stdbool.h"
#include "epaper_adapter.h"
#include "sdkconfig.h"
#include "clock_module.h"
#include "setting_server.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "portmacro.h"


#include "driver/gpio.h"
#include "esp_log.h"


#define ONE_MINUTE      (60*1000)
#define ONE_HOUR        (ONE_MINUTE*60)
#define TIMEOUT_5_SEC  (5*1000)
#define TIMEOUT_6_SEC   (7*1000)
#define TIMEOUT_20_SEC  (20*1000)



enum FuncId{
    SCREEN_MAIN,
    SCREEN_WEATHER_DETAIL,
    SCREEN_TIMER,
    SCREEN_SETTING,
    SCREEN_DEVICE_INFO,
};

struct tm
{
    int	tm_sec;
    int	tm_min;
    int	tm_hour;
    int	tm_mday;
    int	tm_mon;
    int	tm_year;
    int	tm_wday;
    int	tm_yday;
    int	tm_isdst;
};

static void set_next_screen(int cmd);
static void timer_func(int cmd_id);
static void setting_func(int cmd_id);
static void main_func(int cmd_id);
static void device_info_func(int cmd_id);
static void weather_info_func(int cmd_id);


typedef void(*handler_func_t)(int);

static const handler_func_t func_list[] = {
    main_func,
    weather_info_func,
    timer_func,
    setting_func,
    device_info_func,
};

static const int SCREEN_LIST_SIZE = sizeof(func_list)/sizeof(func_list[0]);


enum {
    CMD_INC = BUT_RIGHT,
    CMD_PRESS = BUT_PRESS,
    CMD_DEC = BUT_LEFT,
    CMD_INIT,
    CMD_DEINIT,
    CMD_UPDATE_DATA,
    CMD_UPDATE_POS,
};



static int next_screen;


bool wait_moving(bool expect_moving, int timeout_ms)
{
    while(timeout_ms){
        if(gpio_get_level(GPIO_WAKEUP_PIN) == expect_moving)return true;
        vTaskDelay(100/portTICK_PERIOD_MS);
        timeout_ms -= 100;
    }
    return false;
}





void main_task(void *pv)
{
    set_system_time(60*60*15);
    uint64_t sleep_time_ms;
    unsigned bits;
    int cmd;
    int pos = NO_INP_DATA, pos_data = NO_INP_DATA;
    unsigned screen = -1;
    int off_timeout = 0;
    next_screen = SCREEN_MAIN;
    int min = -1;
    bool update_pos;
    start_timer();

    for(;;){
        update_pos = true;
        service_data.cur_min++;
        do{
            vTaskDelay(200/portTICK_PERIOD_MS);
            bits = device_wait_bits_untile(BIT_WAIT_PROCCESS, 1000/portTICK_PERIOD_MS);

            cmd = device_get_joystick_btn();

            if(cmd == NO_INP_DATA){

                if(bits & BIT_START_MPU_DATA_UPDATE){
                    wait_moving(true, 1000);
                    mpu_measure();
                    pos_data = mpu_get_rotate();
                    device_clear_state(BIT_START_MPU_DATA_UPDATE);
                } else if(get_timer_ms() >= TIMEOUT_5_SEC && update_pos){
                    pos_data = mpu_get_rotate();
                    update_pos = false;
                }
 
                if(pos_data == TURN_DOWN){  
                    if(off_timeout>5){
                        off_timeout = 0;
                        break;
                    }
                    off_timeout += 1;
                } else if(pos_data != pos) {
                    pos = pos_data;
                    off_timeout = 0;
                    if(pos >= 0 && pos < 4){
                        epaper_set_rotate(pos);
                    }
                    cmd = CMD_UPDATE_DATA;
                }

                if(screen != next_screen){
                    if(next_screen >= SCREEN_LIST_SIZE){
                        screen = next_screen = 0;
                    } else if(next_screen < 0){
                        screen = next_screen = SCREEN_LIST_SIZE-1;
                    } else {
                        screen = next_screen;
                    }
                    pos_data = pos;
                    cmd = CMD_INIT;
                } else if(min != service_data.cur_min){
                    min = service_data.cur_min;
                    cmd = CMD_UPDATE_DATA; 
                }
            }

            if(cmd != NO_INP_DATA){
                epaper_clear(UNCOLORED); 
                func_list[screen](cmd);
                bits = device_get_state();  
                epaper_display_all();
            }
            
        }while(screen != next_screen 
                || cmd != NO_INP_DATA
                || get_timer_ms() < TIMEOUT_6_SEC 
                || ( (bits & BIT_WAIT_PROCCESS) && get_timer_ms() < TIMEOUT_20_SEC) );
        if(pos_data == TURN_NORMAL || pos_data == TURN_UP || screen != SCREEN_TIMER){
            next_screen = SCREEN_MAIN;
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
        wifi_off();
        pos_data = mpu_get_rotate();
        if(pos_data == TURN_DOWN){
            sleep_time_ms = ONE_HOUR;
        } else {
            sleep_time_ms = ONE_MINUTE - get_timer_ms();
        }
        if(pos_data == pos){
            device_set_pin(AHT21_EN_PIN, 0);
            device_set_pin(EP_ON_PIN, 0);
            esp_sleep_enable_timer_wakeup(sleep_time_ms * 1000);
            esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_WAKEUP_PIN, 1);
            esp_light_sleep_start();
            device_set_pin(EP_ON_PIN, 1);
            device_set_pin(AHT21_EN_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(500));
            AHT21_init();
            epaper_init();
        }
        start_timer();
    }
}


void service_task(void *pv)
{
    uint32_t bits;
    for(;;){
        bits = device_wait_bits_untile(BIT_START_OPENWEATHER
                            |BIT_START_SERVER, 
                            portMAX_DELAY);
        device_set_state(BIT_WAIT_PROCCESS);
        if (bits & BIT_START_SERVER){
            start_server();
            device_clear_state(BIT_START_SERVER);
        }
        if (bits & BIT_START_OPENWEATHER){
            get_weather(device_get_city_name(), device_get_api_key());
            device_clear_state(BIT_START_OPENWEATHER);
        }
        device_clear_state(BIT_WAIT_PROCCESS);
    }
}



int tasks_init()
{
    if(xTaskCreate(
            service_task, 
            "service",
            10000, 
            NULL, 
            7,
            NULL) != pdTRUE
        || xTaskCreate(
            main_task, 
            "main",
            10000, 
            NULL, 
            5,
            NULL) != pdTRUE 
    ){
        ESP_LOGI("","task create failure");
        return ESP_FAIL;
    }    
    return ESP_OK;   
}



int get_timer_min(int pos)
{
    if(pos == TURN_LEFT || pos == TURN_RIGHT || pos == TURN_UPSIDE_DOWN){
        return pos*10 - 1;
    }
    return 0;
}



static void timer_func(int cmd_id)
{
    static int last_pos;
    static int min_counter = 0, min;
    int pos;
    float t;

    epaper_printf(70, 35, 34, COLORED, "%d:%0.2d", 
                    service_data.cur_min/60, 
                    service_data.cur_min%60);
                    
    if(AHT21_read_data(&t, NULL) == ESP_OK){
        epaper_printf(67, 175, 20, COLORED, "%.1fC*", t);
    }

    pos = mpu_get_rotate();
    if(min_counter){
        if(pos == TURN_NORMAL || pos == TURN_UP){
            wait_moving(true, 4000);
            pos = mpu_get_rotate();
        }
    }


    if(last_pos != pos){
        if(pos == TURN_NORMAL || pos == TURN_UP){
            min_counter = 0;
        }
        if(pos == TURN_LEFT || pos == TURN_RIGHT || pos == TURN_UPSIDE_DOWN){
            min = service_data.cur_min;
            start_single_signale(10, 1000);
            min_counter = get_timer_min(pos);
        }
        last_pos = pos;
    }

    set_next_screen(cmd_id);

    if(min_counter){
        if(service_data.cur_min != min){
            min_counter -= 1;
            min = service_data.cur_min;
            if(min_counter == 0){
                epaper_refresh();
                start_alarm();
                vTaskDelay(2000/portTICK_PERIOD_MS);
                wait_moving(false, 5000);
                min_counter = get_timer_min(mpu_get_rotate());
            }
        }
    }

    if(min_counter){
        epaper_printf(70, 75, 64, COLORED, "%i", min_counter);
        epaper_print_str(85, 135, 24, COLORED, "min");
    } else {
        epaper_print_str(30, 90, 48, COLORED, "Stop");
    }
}


static void set_next_screen(int cmd)
{
    if(cmd == CMD_INC){
        ++next_screen;
    } else if(cmd == CMD_DEC){
        --next_screen;
    }  
}


void setting_func(int cmd_id)
{
    ESP_LOGI("",  "setting func");
    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
    }else if(cmd_id == CMD_INIT){
        epaper_print_str(20,40,24, COLORED, "press button for starting");
        epaper_print_str(50,40,24, COLORED, "settings server");
    } else if(cmd_id == CMD_PRESS) {
        epaper_print_str(20, 10, 24, COLORED, "Settings server run");
        epaper_print_str(110, 20,24, COLORED, "http://192.168.4.1/");
        epaper_print_str(50, 5, 20, COLORED, "SSID:" CONFIG_WIFI_AP_SSID);
        epaper_print_str(80, 5, 20, COLORED, "Password:" CONFIG_WIFI_AP_PASSWORD);
        device_set_state(BIT_START_SERVER);
    }
}


void main_func(int cmd_id)
{
    ESP_LOGI("",  "main func");
    float t, hum;
    int pos = mpu_get_rotate();
    if(pos == TURN_UP){
        next_screen = SCREEN_WEATHER_DETAIL;
    } else if(pos == TURN_LEFT || pos == TURN_RIGHT || pos == TURN_UPSIDE_DOWN){
        next_screen = SCREEN_TIMER;
    } else if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
    } else {
        if(AHT21_read_data(&t, &hum) == ESP_OK){
            draw_horizontal_line(0, 200, 70, 2, COLORED);
            epaper_print_str(5, 40, 20, COLORED, "room");
            epaper_printf(90, 25, 24, COLORED, "%.1fC*", t);
            epaper_printf(90, 50, 24, COLORED, "%.1f%%", hum);
        }
        epaper_printf(60, 75, 24, COLORED, "%.1f C*", service_data.temp_list[0]);
        epaper_print_str(10, 100, 20, COLORED, service_data.desciption);
        epaper_printf(30, 125, 48, COLORED, snprintf_time("%H:%M"));
        epaper_printf(50, 172, 24, COLORED, snprintf_time("%d %a"));
    }
    if(cmd_id == CMD_PRESS){
        device_set_state(BIT_START_OPENWEATHER);
    }
}


void device_info_func(int cmd_id)
{

        unsigned bits = device_get_state();
        epaper_printf(10, 30, 20, COLORED, "Battery: %.2fd V", adc_reader_get_voltage());
        epaper_printf(10, 55, 20, COLORED, "WIFi: %s", 
                            bits&BIT_IS_STA_CONNECTION
                            ? "ok"
                            : "nok");
        epaper_printf(10, 80, 20, COLORED, "SNTP: %s", 
                            bits&BIT_SNTP_OK
                            ? "ok"
                            : "nok");
        epaper_printf(10, 105, 20, COLORED, "Openweather: %s", 
                            bits&BIT_OPENWEATHER_OK
                            ? "ok"
                            : "nok");
    set_next_screen(cmd_id);
}


void weather_info_func(int cmd_id)
{
    if(cmd_id == CMD_PRESS){
        device_set_state(BIT_START_OPENWEATHER);
    } else {
        set_next_screen(cmd_id);
    }
    epaper_print_str(10, 15, 24, COLORED, service_data.desciption);
    unsigned h = service_data.cur_min / 60;
    for(int i=0; i<sizeof(service_data.temp_list)/sizeof(service_data.temp_list[0]); ++i){
        if(h>23)h %= 24;
        epaper_printf(10, 30+i*25, 24, COLORED, "%c%d:00  %.1fC*", 
                            h>10 ? h/10+'0':' ', 
                            h%10, 
                            service_data.temp_list[i]);
        h += 3;
    }
}









