#include "network_service.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "sdkconfig.h"
#include "esp_mac.h"
#include "additional_functions.h"


EventGroupHandle_t service_event_group;
static wifi_config_t wifi_sta_config;
static wifi_config_t wifi_ap_config;
static esp_netif_t *netif;
static wifi_mode_t mode;


static EventBits_t wait_bits(EventBits_t bits)
{
    return xEventGroupWaitBits(service_event_group,
                bits,
                pdFALSE,
                pdFALSE,
                10000/portTICK_PERIOD_MS);
}


static void sta_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    static int retry_num;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        retry_num = 0;
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if(retry_num < 5){
            esp_wifi_connect();
            ++retry_num;
            wifi_event_sta_disconnected_t *event_sta_disconnected = (wifi_event_sta_disconnected_t *) event_data;
            if(event_sta_disconnected->reason == WIFI_REASON_NO_AP_FOUND
                        || event_sta_disconnected->reason == WIFI_REASON_HANDSHAKE_TIMEOUT){
                xEventGroupSetBits(service_event_group, BIT_SSID_NO_FOUND);
            } else {
                xEventGroupClearBits(service_event_group, BIT_SSID_NO_FOUND);
            }
        } else {
            xEventGroupClearBits(service_event_group, BIT_STA_CON);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        retry_num = 0;
        xEventGroupSetBits(service_event_group, BIT_STA_CON);
        xEventGroupClearBits(service_event_group, BIT_SSID_NO_FOUND);
    }
}


static void ap_handler(void* main_data, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{      
    if (event_id == WIFI_EVENT_AP_STOP){
        
    } else if(event_id == WIFI_EVENT_AP_STACONNECTED){
        // wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        // event->mac;
    } else if(event_id == WIFI_EVENT_AP_STADISCONNECTED){

    }
}


int init_wifi(void) 
{
    // if(service_event_group == NULL){
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        service_event_group = xEventGroupCreate();
        mode = WIFI_MODE_NULL;
        esp_err_t ret = nvs_flash_init();

        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            CHECK_AND_RET_ERR(nvs_flash_erase());
            CHECK_AND_RET_ERR(nvs_flash_init());
        }
        CHECK_AND_RET_ERR(esp_netif_init());
        CHECK_AND_RET_ERR(esp_event_loop_create_default());
        CHECK_AND_RET_ERR(esp_wifi_init(&cfg));
    // }
    return ESP_OK;
}



void set_wifi_sta_config(char *ssid, char *pwd)
{
    memset(&wifi_sta_config, 0, sizeof(wifi_sta_config));
    strncpy((char *)wifi_sta_config.sta.ssid, ssid, 32);
    strncpy((char *)wifi_sta_config.sta.password, pwd, 32);
    wifi_sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    // wifi_sta_config.sta.sae_pwe_h2e = WIFI_AUTH_WPA2_PSK;
}


void set_wifi_ap_config(char *ssid, char *pwd)
{
    // if(service_event_group == NULL){
    //     init_wifi();
    // }
    memset(&wifi_ap_config, 0, sizeof(wifi_ap_config));
    strncpy((char *)wifi_ap_config.sta.ssid, ssid, 32);
    strncpy((char *)wifi_ap_config.sta.password, pwd, 32);
    wifi_ap_config.ap.max_connection = CONFIG_MAX_STA_CONN;
    wifi_ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_ap_config.ap.channel = 1;
    wifi_ap_config.ap.pmf_cfg.required = false;
}




int connect_sta()
{
    if(service_event_group == NULL){
        CHECK_AND_RET_ERR(init_wifi());
    }
    if(mode == WIFI_MODE_STA){
        esp_wifi_stop();
    } else {
        if(mode != WIFI_MODE_NULL){
            wifi_stop(); 
        }
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_STOP, &sta_handler, NULL));
    }
    if(netif == NULL){
       netif = esp_netif_create_default_wifi_sta();
       if(netif == NULL) return ESP_FAIL;
    }

    mode = WIFI_MODE_STA;
    CHECK_AND_RET_ERR(esp_wifi_set_mode(WIFI_MODE_STA));
#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
        CHECK_AND_RET_ERR(esp_wifi_set_protocol(ESP_IF_WIFI_STA,
                                                WIFI_PROTOCOL_11B
                                                |WIFI_PROTOCOL_11G
                                                |WIFI_PROTOCOL_11N
                                                |WIFI_PROTOCOL_LR));
#endif
    if(strnlen((char*)wifi_sta_config.sta.ssid, sizeof(wifi_sta_config.sta.ssid)) == 0){
        set_wifi_sta_config(CONFIG_WIFI_STA_DEBUG_SSID, CONFIG_WIFI_STA_DEBUG_PASSWORD);
    }
    CHECK_AND_RET_ERR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    CHECK_AND_RET_ERR(esp_wifi_start());
    return wait_bits(BIT_STA_CON)&BIT_STA_CON ? ESP_OK : ESP_ERR_TIMEOUT;
}


int connect_ap()
{
    if(service_event_group == NULL){
        CHECK_AND_RET_ERR(init_wifi());
    }
    if(mode != WIFI_MODE_AP){
        if(mode != WIFI_MODE_NULL){
            wifi_stop(); 
        }
        if(netif){
            esp_netif_destroy_default_wifi(netif);
            netif = NULL;  
        }
        netif = esp_netif_create_default_wifi_ap();
        if(netif == NULL) return ESP_FAIL;

        if(strnlen((char *)wifi_ap_config.ap.ssid, sizeof(wifi_ap_config.ap.ssid)) == 0){
            set_wifi_ap_config(CONFIG_WIFI_AP_SSID, CONFIG_WIFI_AP_PASSWORD);
        }
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &ap_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &ap_handler, NULL));       
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &ap_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &ap_handler, NULL));
        mode = WIFI_MODE_AP;
        CHECK_AND_RET_ERR(esp_wifi_set_mode(WIFI_MODE_AP));
        CHECK_AND_RET_ERR(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    }
    return esp_wifi_start();
}


void wifi_stop()
{
    esp_wifi_stop();
    vTaskDelay(500);
    if(mode == WIFI_MODE_AP){
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_START, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STOP, &ap_handler);
    } else if(mode == WIFI_MODE_STA) {
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, &sta_handler);
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_STOP, &sta_handler);
    }
    mode = WIFI_MODE_NULL;
    if(netif){
        // esp_netif_destroy_default_wifi(netif);
        netif = NULL;
    }
}


static void set_time_cb(struct timeval *tv)
{
    const int INTERVAL_10_HOUR = 1000*60*60*10;
    settimeofday(tv, NULL);
    setenv("TZ", "EET2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();
    // if the first call
    if(esp_sntp_get_sync_interval() < INTERVAL_10_HOUR){
        esp_sntp_set_sync_interval(INTERVAL_10_HOUR);
        xEventGroupSetBits(service_event_group, BIT_TIME_OK|BIT_SNTP_OK);
    }
}



void init_sntp()
{
    if(esp_sntp_enabled()){
        esp_sntp_restart();
    } else {
        esp_sntp_set_time_sync_notification_cb(set_time_cb);
        esp_sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
        esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_setservername(1, "time.windows.com");
        sntp_servermode_dhcp(0);
        const int INTERVAL_1_MIN = 1000*60*1;
        esp_sntp_set_sync_interval(INTERVAL_1_MIN);
        esp_sntp_init();
    }
}


void stop_sntp()
{
    esp_sntp_stop();
    if(service_event_group){
        xEventGroupClearBits(service_event_group, BIT_SNTP_OK);
    }
}

bool get_time(char *strftime_buf, int buf_size)
{
    if(service_event_group == NULL && !init_wifi() )return false;
    EventBits_t bits = xEventGroupGetBits(service_event_group);
    if( !(bits&BIT_STA_CON) ){
        if(connect_sta() != ESP_OK)return false;
    }
    if(!(bits&BIT_SNTP_OK)){
        init_sntp();
        if(!(wait_bits(BIT_SNTP_OK)&BIT_SNTP_OK))return false;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, buf_size, "%H:%M:%S", &timeinfo);
    return timeinfo.tm_year > (2016 - 1900);
}