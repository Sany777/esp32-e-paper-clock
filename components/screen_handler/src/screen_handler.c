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

#define TIMEOUT_SEC     (1000)
#define TIMEOUT_2_SEC   (2*TIMEOUT_SEC)
#define ONE_MINUTE      (60*TIMEOUT_SEC)
#define TIMEOUT_4_HOUR  (240*ONE_MINUTE)
#define TIMEOUT_5_SEC   (5*TIMEOUT_SEC)
#define TIMEOUT_7_SEC   (7*TIMEOUT_SEC)
#define TIMEOUT_6_SEC   (6*TIMEOUT_SEC)
#define TIMEOUT_20_SEC  (20*TIMEOUT_SEC)
#define HALF_MINUTE     (30*TIMEOUT_SEC)


enum FuncId{
    SCREEN_MAIN,
    SCREEN_BROADCAST_DETAIL,
    SCREEN_TIMER,
    SCREEN_SETTING,
    SCREEN_DEVICE_INFO,
};



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
    unsigned bits;
    int cmd;
    int min = -1;
    int pos_data, last_pos = TURN_NORMAL;
    int screen = -1;
    uint64_t sleep_time_ms;
    next_screen = SCREEN_MAIN;
    bool but_input, exit, refresh = true;
    int timeout = TIMEOUT_6_SEC;
    vTaskDelay(100/portTICK_PERIOD_MS);
    struct tm * timeinfo;
    for(;;){

        bits = device_get_state();

        if( !(bits & BIT_IS_TIME) ){
            device_set_state(BIT_UPDATE_BROADCAST_DATA);
        } 

        if( !(bits & BIT_BROADCAST_OK)){
            device_set_state(BIT_UPDATE_BROADCAST_DATA);
        }

        but_input = exit = false;
        start_timer();
        do{
            vTaskDelay(100/portTICK_PERIOD_MS);

            cmd = device_get_joystick_btn();

            if(cmd != NO_DATA){
                if(!but_input){
                    but_input = true;
                } else if(cmd == CMD_PRESS){
                    ++next_screen;
                }
            }

            if(but_input){
                bits = device_get_state();
            } else {
                bits = device_wait_bits_untile(BIT_WAIT_MOVING, 3000/portTICK_PERIOD_MS);
            }

            pos_data = mpu_get_rotate();

            if(pos_data != last_pos){
                if(pos_data == TURN_DOWN)
                     continue;
                epaper_set_rotate(pos_data);
                last_pos = pos_data;
                cmd = CMD_UPDATE_POS; 
            }

            if(but_input){
                pos_data = NO_DATA;
            }

            timeinfo = get_time_tm();
            service_data.cur_min = get_time_in_min(timeinfo);

            if(screen != next_screen) {
                if(next_screen >= SCREEN_LIST_SIZE){
                    next_screen = 0;
                } else if(next_screen < 0){
                    next_screen = SCREEN_LIST_SIZE-1;
                }
                screen = next_screen;
                cmd = CMD_INIT;
            } else if(bits & BIT_NEW_DATA) {
                cmd = CMD_UPDATE_DATA;
                device_clear_state(BIT_NEW_DATA);
            } else if(min != service_data.cur_min) {
                min = service_data.cur_min;
                cmd = CMD_UPDATE_DATA; 
                if(is_signale(service_data.cur_min, timeinfo->tm_wday));
            } else if(!exit && get_timer_ms() > timeout) {
                exit = true;
                cmd = CMD_UPDATE_DATA;
                refresh = true;
            }
            
            if(cmd != NO_DATA){
                epaper_clear(UNCOLORED); 
                func_list[screen](cmd, pos_data);
                bits = device_get_state();
                if(screen == next_screen){
                    if(refresh){
                        refresh = false;
                        epaper_display_all();
                    } else {
                        epaper_display_part();
                    }
                }
            }
            
        } while(!exit 
                || cmd != NO_DATA 
                || bits & BIT_WAIT_PROCCESS
                || bits & BIT_WAIT_BUT_INPUT);
        
        
        vTaskDelay(100/portTICK_PERIOD_MS);
        wifi_stop();
        device_clear_state(BIT_IS_AP_MODE|BIT_IS_STA_CONNECTION);
        epaper_wait();
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
            if(wait_moving(true, 500)){
                timeout = TIMEOUT_6_SEC;
            } else {
                timeout = TIMEOUT_SEC;
            }
            AHT21_init();
            epaper_init();
        }
    }
}



void service_task(void *pv)
{
    uint32_t bits;
    int timeout = 0;
    bool open_sesion = false;
    vTaskDelay(100/portTICK_PERIOD_MS);
    for(;;){
        device_wait_bits_untile(BIT_UPDATE_BROADCAST_DATA
                            | BIT_INIT_SNTP
                            | BIT_START_SERVER, 
                            portMAX_DELAY);

        bits = device_set_state(BIT_WAIT_PROCCESS);

        if(bits & BIT_START_SERVER){
            if(start_ap() == ESP_OK && init_server(network_buf) == ESP_OK){
                device_clear_state(BIT_IS_STA_CONNECTION);
                device_set_state(BIT_SERVER_RUN|BIT_NEW_DATA);
                open_sesion = false;
                while(bits = device_get_state(), bits&BIT_SERVER_RUN){
                    if(open_sesion){
                        if(!(bits&BIT_IS_AP_CONNECTION) ){
                            device_clear_state(BIT_SERVER_RUN);
                        }
                    } else if(bits&BIT_IS_AP_CONNECTION){
                        open_sesion = true;
                    } else if(timeout>600){
                        device_clear_state(BIT_SERVER_RUN);
                    } else {
                        timeout += 1;
                    }
                    vTaskDelay(100/portTICK_PERIOD_MS);
                }
                deinit_server();
                device_commit_changes();
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
            device_clear_state(BIT_START_SERVER);
            bits = device_set_state(BIT_NEW_DATA);
        }

        if(bits&BIT_UPDATE_BROADCAST_DATA || bits&BIT_INIT_SNTP){
            if(connect_sta(device_get_ssid(),device_get_pwd()) == ESP_OK){
                if(bits&BIT_INIT_SNTP){
                    init_sntp();
                    bits = device_clear_state(BIT_INIT_SNTP);
                }
                if(bits&BIT_UPDATE_BROADCAST_DATA){
                    if(get_weather(device_get_city_name(),device_get_api_key()) == ESP_OK){
                        device_set_state(BIT_BROADCAST_OK|BIT_NEW_DATA);
                    } else {
                        device_clear_state(BIT_BROADCAST_OK);
                    }
                    device_clear_state(BIT_UPDATE_BROADCAST_DATA);
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
            15000, 
            NULL, 
            5,
            NULL) != pdTRUE
        || xTaskCreate(
            main_task, 
            "main",
            15000, 
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
    return 5;
}



static void timer_func(int cmd_id, int pos_data)
{
    static int min_counter = 0, min, init_min = 0;
    float t;
    bool pausa;

    if(pos_data != NO_DATA){

        pausa = pos_data == TURN_UP;

        if(pos_data == TURN_NORMAL){
            next_screen = SCREEN_MAIN;
            return;
        }

        if(cmd_id == CMD_UPDATE_POS || cmd_id == CMD_INIT){
            start_single_signale(10, 1000);
            min_counter = get_timer_min(pos_data);
        }

    } else {
        pausa = TURN_UP == mpu_get_rotate();
    }

    if(cmd_id == CMD_INC){
        init_min = min_counter = (init_min/5)*5 + 5;
    } else if(cmd_id == CMD_DEC){
        if(init_min){
            if(init_min <= 5){
                init_min -= 1;
            } else {
                init_min = (init_min/5)*5 - 5;
            }
        }
        min_counter = init_min;
    }

    epaper_printf(70, 35, 34, COLORED, "%d:%0.2d", 
                    service_data.cur_min/60, 
                    service_data.cur_min%60);
                    
    if(AHT21_read_data(&t, NULL) == ESP_OK){
        epaper_printf(67, 175, 20, COLORED, "%.1fC*", t);
    }
    

    if(min_counter){
        if(service_data.cur_min != min){
            min = service_data.cur_min;
            if(!pausa){
                min_counter -= 1;
                if(min_counter == 0){
                    epaper_refresh();
                    start_alarm();
                    vTaskDelay(2000/portTICK_PERIOD_MS);
                    wait_moving(true, 5000);
                    if(init_min){
                        min_counter = init_min;
                    } else {
                        min_counter = mpu_get_rotate();
                    }
                }
            }
        }
    }

    if(min_counter){
        epaper_printf(70, 75, 64, COLORED, "%i", min_counter);
        epaper_print_str(85, 135, 20, COLORED, "min");
        if(pausa){
            epaper_print_str(115, 135, 20, COLORED, "Pausa");
        }
    } else {
        epaper_print_str(30, 90, 48, COLORED, "Stop");
    }  
}


void setting_func(int cmd_id, int pos_data)
{
    ESP_LOGI("",  "setting func");
    unsigned bits = device_get_state();

    if(bits&BIT_SERVER_RUN){
        if(cmd_id == CMD_INC){
            device_clear_state(BIT_SERVER_RUN|BIT_START_SERVER);
        }
        epaper_print_str(10, 20, 16, COLORED, "Server run!");
        epaper_print_str(3, 40, 16, COLORED, "http://192.168.4.1");
        epaper_print_str(3, 60, 16, COLORED, "SSID:" CONFIG_WIFI_AP_SSID);
        epaper_print_str(3, 80, 16, COLORED, "Password:" CONFIG_WIFI_AP_PASSWORD);
    } else {
        if(cmd_id == CMD_DEC){
            device_set_state(BIT_START_SERVER|BIT_WAIT_PROCCESS);
        }
        epaper_print_str(30,40,16, COLORED, "Press button");
        epaper_print_str(30,60,16, COLORED, "for starting");
        epaper_print_str(30,80,16, COLORED, "settings server");
    }

}


void main_func(int cmd_id, int pos_data)
{
    ESP_LOGI("",  "main func");
    float t, hum;

    if(pos_data == TURN_UP){
        next_screen = SCREEN_BROADCAST_DETAIL;
        return;
    } 
    if(pos_data == TURN_LEFT || pos_data == TURN_RIGHT || pos_data == TURN_UPSIDE_DOWN){
        next_screen = SCREEN_TIMER;
        return;
    }

    if(cmd_id == CMD_DEC || cmd_id == CMD_INC){
        device_set_state(BIT_UPDATE_BROADCAST_DATA);
    }
    if(adc_reader_get_voltage() < 3.5f){
        epaper_printf(10, 10, 16, COLORED, "BAT!");
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
}


void device_info_func(int cmd_id, int pos_data)
{
    unsigned bits = device_get_state();
    draw_horizontal_line(0, 200, 52, 2, COLORED);
    epaper_printf(5, 30, 20, COLORED, "Battery: %.2fV", adc_reader_get_voltage());
    draw_horizontal_line(0, 200, 30, 5, COLORED);
    epaper_printf(5, 55, 20, COLORED, "STA: %s", 
                        bits&BIT_IS_STA_CONNECTION
                        ? "con."
                        : "disc.");
    epaper_printf(5, 55, 20, COLORED, "AP: %s", 
                        bits&BIT_IS_AP_CONNECTION
                        ? "con."
                        : "disc.");
    epaper_printf(5, 80, 20, COLORED, "SNTP: %s", 
                        bits&BIT_SNTP_OK
                        ? "ok"
                        : "nok");
    epaper_printf(5, 105, 20, COLORED, "Openweath.:%s", 
                        bits&BIT_BROADCAST_OK
                        ? "ok"
                        : "nok");
}


void weather_info_func(int cmd_id, int pos_data)
{
    ESP_LOGI("",  "weather info func");

    if(pos_data == TURN_NORMAL){
        next_screen = SCREEN_MAIN;
        return;
    }else if(pos_data == TURN_LEFT || pos_data == TURN_RIGHT){
        next_screen = SCREEN_TIMER;
        return;
    }else if(pos_data == TURN_UPSIDE_DOWN){
        next_screen = SCREEN_SETTING;
        return;
    }

    if(cmd_id == CMD_INC || cmd_id == CMD_DEC){
        device_set_state(BIT_UPDATE_BROADCAST_DATA);
    }
    epaper_print_str(10, 40, 24, COLORED, service_data.desciption);
    unsigned h = service_data.cur_min / 60;
    for(int i=0; i<TEMP_LIST_SIZE; ++i){
        if(h>23)h %= 24;
        epaper_printf(10, 70+i*25, 24, COLORED, "%c%d:00  %.1fC*", 
                            h>10 ? h/10+'0':' ', 
                            h%10, 
                            service_data.temp_list[i]);
        h += 3;
    }
}









