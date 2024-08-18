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

#define TIMEOUT_2_SEC   (2000)
#define ONE_MINUTE      (60*1000)
#define TIMEOUT_4_HOUR  (ONE_MINUTE*240)
#define TIMEOUT_5_SEC   (5*1000)
#define TIMEOUT_6_SEC   (7*1000)
#define TIMEOUT_20_SEC  (20*1000)
#define HALF_MINUTE     (30*1000)


enum FuncId{
    SCREEN_MAIN,
    SCREEN_BROADCAST_DETAIL,
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

static void set_next_screen(int cmd_id);
static void timer_func(int cmd_id, int pos_data);
static void setting_func(int cmd_id, int pos_data);
static void main_func(int cmd_id, int pos_data);
static void device_info_func(int cmd_id, int pos_data);
static void weather_info_func(int cmd_id, int pos_data);


typedef void(*handler_func_t)(int, int);

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
    CMD_UPDATE_TIME,
    CMD_UPDATE_POS,
};



static int next_screen;


bool wait_moving(bool wait_move, int timeout_ms)
{
    int val = !wait_move;
    while(timeout_ms){
        if(gpio_get_level(GPIO_WAKEUP_PIN) == val)
            return true;
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
    int min = -1;
    int pos_data, last_pos = TURN_NORMAL;
    int screen = NO_DATA;
    next_screen = SCREEN_MAIN;
    bool first_init, last_init;
    for(;;){
        start_timer();
        service_data.cur_min++;
        first_init = last_init = true;
        do{
            vTaskDelay(200/portTICK_PERIOD_MS);

            bits = device_wait_bits_untile(BIT_WAIT_MOVING, 4000/portTICK_PERIOD_MS);

            pos_data = mpu_get_rotate();

            cmd = device_get_joystick_btn();

            if(cmd == NO_DATA){

                if(bits & BIT_NEW_DATA){
                    cmd = CMD_UPDATE_DATA;
                    device_clear_state(BIT_NEW_DATA);
                }else if(last_init && get_timer_ms() > TIMEOUT_5_SEC){
                    last_init = false;
                    if(pos_data == TURN_NORMAL && screen != SCREEN_MAIN){
                        next_screen = SCREEN_MAIN;
                        cmd = CMD_INIT;
                    } else {
                        cmd = CMD_UPDATE_DATA;
                    }
                } else if(first_init && get_timer_ms() > TIMEOUT_2_SEC){
                    first_init = false;
                    cmd = CMD_UPDATE_DATA;
                }else if(screen != next_screen){
                    if(next_screen >= SCREEN_LIST_SIZE){
                        screen = next_screen = 0;
                    } else if(next_screen < 0){
                        screen = next_screen = SCREEN_LIST_SIZE-1;
                    } else {
                        screen = next_screen;
                    }
                    cmd = CMD_INIT;
                }else if(pos_data != last_pos){
                    wait_moving(true, 3000);
                    pos_data = mpu_get_rotate();
                    if(pos_data != last_pos){
                        ESP_LOGI("mpu", "%s", mpu_pos_to_str(pos_data));
                        if(pos_data != TURN_DOWN){
                            cmd = CMD_UPDATE_POS; 
                            last_pos = pos_data;
                        }
                    }
                }else if(min != service_data.cur_min){
                    min = service_data.cur_min;
                    cmd = CMD_UPDATE_DATA; 
                }
            }

            if(cmd != NO_DATA){
                epaper_set_rotate(pos_data);
                epaper_clear(UNCOLORED); 
                if(screen >= sizeof(func_list)/sizeof(func_list[0])){
                    screen = next_screen = SCREEN_MAIN;
                }
                func_list[screen](cmd, pos_data);
                bits = device_get_state();  
            }
            
        }while( cmd != NO_DATA 
                || last_init
                || get_timer_ms() < TIMEOUT_6_SEC 
                || bits & BIT_WAIT_PROCCESS
                || bits & BIT_BUT_INPUT);

        vTaskDelay(100/portTICK_PERIOD_MS);
        wifi_off();
        
        if(pos_data == mpu_get_rotate()){

            const int working_time = get_timer_ms();

            if(pos_data == TURN_DOWN){
                sleep_time_ms = TIMEOUT_4_HOUR;
            } else if( (ONE_MINUTE - working_time) > 100){
                sleep_time_ms = ONE_MINUTE - working_time;
            } else {
                sleep_time_ms = ONE_MINUTE - TIMEOUT_6_SEC;
            }
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
    }
}



void service_task(void *pv)
{
    uint32_t bits;
    for(;;){
        bits = device_wait_bits_untile(BIT_UPDATE_BROADCAST_DATA
                            | BIT_INIT_SNTP
                            | BIT_START_SERVER, 
                            portMAX_DELAY);

        bits = device_set_state(BIT_WAIT_PROCCESS);

        if(bits & BIT_START_SERVER){
            start_server();
            device_clear_state(BIT_START_SERVER|BIT_IS_STA_CONNECTION);
        }

        if(bits&BIT_UPDATE_BROADCAST_DATA || bits&BIT_INIT_SNTP){
            if( !(bits&BIT_STA_CONF_OK)){
                set_wifi_sta_config(device_get_ssid(), device_get_pwd());
                bits = device_get_state();
            }
            if( bits&BIT_STA_CONF_OK && !(bits&BIT_IS_STA_CONNECTION)){
                connect_sta();
                bits = device_get_state();
            }
            if(bits&BIT_IS_STA_CONNECTION){
                if(bits&BIT_INIT_SNTP){
                    init_sntp();
                }
                if(bits&BIT_UPDATE_BROADCAST_DATA){
                    get_weather(device_get_city_name(), device_get_api_key());
                }
            }
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



static void timer_func(int cmd_id, int pos_data)
{
    static int min_counter = 0, min;
    bool pausa = pos_data == TURN_UP;
    float t;

    if(pos_data == TURN_NORMAL){
        next_screen = SCREEN_MAIN;
        return;
    }

    if(cmd_id == CMD_UPDATE_POS || min_counter == 0 || cmd_id == CMD_INIT){
        start_single_signale(10, 1000);
        min_counter = get_timer_min(pos_data);
    } 
    
    if(cmd_id == CMD_INC || cmd_id == CMD_PRESS){
        min_counter = (min_counter/5)*5 + 5;
    } else if(cmd_id == CMD_DEC){
        min_counter = (min_counter/5)*5 - 5;
    }

    epaper_printf(70, 35, 34, COLORED, "%d:%0.2d", 
                    service_data.cur_min/60, 
                    service_data.cur_min%60);
                    
    if(AHT21_read_data(&t, NULL) == ESP_OK){
        epaper_printf(67, 175, 20, COLORED, "%.1fC*", t);
    }
    if(pausa){
        epaper_print_str(30, 90, 48, COLORED, "Pausa");
    } else {
        if(min_counter){
            if(service_data.cur_min != min){
                min_counter -= 1;
                min = service_data.cur_min;
                if(min_counter == 0){
                    epaper_refresh();
                    start_alarm();
                    vTaskDelay(2000/portTICK_PERIOD_MS);
                    wait_moving(true, 5000);
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

    epaper_display_all();
}


static void set_next_screen(int cmd)
{
    if(cmd == CMD_INC){
        ++next_screen;
    } else if(cmd == CMD_DEC){
        --next_screen;
    }  
}


void setting_func(int cmd_id, int pos_data)
{
    ESP_LOGI("",  "setting func");
    static bool started;

    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
        return;
    }
    
    if(cmd_id == CMD_PRESS){
        if(started){
            device_clear_state(BIT_SERVER_RUN);
        } else {
            device_set_state(BIT_START_SERVER|BIT_WAIT_PROCCESS);
        }
        started = !started;
    }

    if(started){
        epaper_print_str(10, 20, 16, COLORED, "Server run!");
        epaper_print_str(3, 40, 16, COLORED, "http://192.168.4.1");
        epaper_print_str(3, 60, 16, COLORED, "SSID:" CONFIG_WIFI_AP_SSID);
        epaper_print_str(3, 80, 16, COLORED, "Password:" CONFIG_WIFI_AP_PASSWORD);
    } else {
        epaper_print_str(30,40,16, COLORED, "Press button");
        epaper_print_str(30,60,16, COLORED, "for starting");
        epaper_print_str(30,80,16, COLORED, "settings server");
    }
    epaper_display_all();
}


void main_func(int cmd_id, int pos_data)
{
    ESP_LOGI("",  "main func");
    float t, hum;

    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
        return;
    }
    if(pos_data == TURN_UP){
        next_screen = SCREEN_BROADCAST_DETAIL;
        return;
    } 
    if(pos_data == TURN_LEFT || pos_data == TURN_RIGHT || pos_data == TURN_UPSIDE_DOWN){
        next_screen = SCREEN_TIMER;
        return;
    }

    if(cmd_id == CMD_PRESS){
        device_set_state(BIT_UPDATE_BROADCAST_DATA);
    }

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

    epaper_display_all();
}


void device_info_func(int cmd_id, int pos_data)
{
    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
        return;
    } 

    unsigned bits = device_get_state();
    draw_horizontal_line(0, 200, 52, 2, COLORED);
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
                        bits&BIT_BROADCAST_OK
                        ? "ok"
                        : "nok");
    epaper_display_all();
    
}


void weather_info_func(int cmd_id, int pos_data)
{
    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        set_next_screen(cmd_id);
        return;
    } 

    if(pos_data != TURN_UP){
        if(pos_data == TURN_NORMAL){
            next_screen = SCREEN_MAIN;
        }else if(pos_data == TURN_LEFT || pos_data == TURN_RIGHT){
            next_screen = SCREEN_TIMER;
        }else if(pos_data == TURN_UPSIDE_DOWN){
            next_screen = SCREEN_SETTING;
        }
        return;
    }

    if(cmd_id == CMD_PRESS){
        device_set_state(BIT_UPDATE_BROADCAST_DATA);
    }
    epaper_printf(5, 10, 20, COLORED, "Battery:%.2fv", adc_reader_get_voltage());
    epaper_print_str(10, 40, 24, COLORED, service_data.desciption);
    unsigned h = service_data.cur_min / 60;
    for(int i=0; i<sizeof(service_data.temp_list)/sizeof(service_data.temp_list[0]); ++i){
        if(h>23)h %= 24;
        epaper_printf(10, 70+i*25, 24, COLORED, "%c%d:00  %.1fC*", 
                            h>10 ? h/10+'0':' ', 
                            h%10, 
                            service_data.temp_list[i]);
        h += 3;
    }
    epaper_display_all();
    
}









