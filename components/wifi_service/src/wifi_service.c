#include "wifi_service.h"

#include "device_system.h"
#include <stdio.h>
#include <string.h>


#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "sdkconfig.h"
#include "esp_mac.h"
#include "additional_functions.h"
#include "device_system.h"
#include "setting_server.h"
#include "device_macro.h"

#include "string.h"
#define MIN_WIFI_PWD_LEN  8
#define MIN_WIFI_SSID_LEN 1 


wifi_mode_t wifi_mode;

static wifi_config_t wifi_sta_config;
static wifi_config_t wifi_ap_config;
static esp_netif_t *netif;




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
                device_set_state(BIT_ERR_SSID_NO_FOUND);
            } else {
                device_clear_state(BIT_ERR_SSID_NO_FOUND);
            }
        } else {
            device_clear_state(BIT_IS_STA_CONNECTION);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        retry_num = 0;
        device_set_state(BIT_IS_STA_CONNECTION);
        device_clear_state(BIT_ERR_SSID_NO_FOUND);

    }
}


static void ap_handler(void* main_data, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STOP){

    } else if(event_id == WIFI_EVENT_AP_STACONNECTED){
        device_set_state(BIT_IS_AP_CONNECTION);
    } else if(event_id == WIFI_EVENT_AP_STADISCONNECTED){
        device_clear_state(BIT_IS_AP_CONNECTION);
    }
}

void wifi_off()
{
    if(device_get_state()&BIT_IS_WIFI_INIT){
        wifi_stop();
        esp_wifi_deinit();
        device_clear_state(BIT_IS_WIFI_INIT);
    }
}


int wifi_init(void) 
{
    if(device_get_state()&BIT_IS_WIFI_INIT)return ESP_FAIL;
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_mode = WIFI_MODE_NULL;
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        CHECK_AND_RET_ERR(nvs_flash_erase());
        CHECK_AND_RET_ERR(nvs_flash_init());
    }
    CHECK_AND_RET_ERR(esp_event_loop_create_default());
    CHECK_AND_RET_ERR(esp_netif_init());
    CHECK_AND_RET_ERR(esp_wifi_init(&cfg));
    CHECK_AND_RET_ERR(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    device_set_state(BIT_IS_WIFI_INIT);
    return ESP_OK;
}


int set_wifi_sta_config(char *ssid, char *pwd)
{
    if(strlen(ssid) >= MIN_WIFI_SSID_LEN
            && strlen(pwd) >= MIN_WIFI_PWD_LEN){
        memset(&wifi_sta_config, 0, sizeof(wifi_sta_config));
        strncpy((char *)wifi_sta_config.sta.ssid, ssid, 32);
        strncpy((char *)wifi_sta_config.sta.password, pwd, 32);
        wifi_sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        device_set_state(BIT_STA_CONF_OK);
        return ESP_OK;
    }
    return ESP_FAIL;
}


void set_wifi_ap_config(char *ssid, char *pwd)
{
    memset(&wifi_ap_config, 0, sizeof(wifi_ap_config));
    strncpy((char *)wifi_ap_config.sta.ssid, ssid, sizeof(wifi_ap_config.sta.ssid));
    strncpy((char *)wifi_ap_config.sta.password, pwd, sizeof(wifi_ap_config.sta.password));
    wifi_ap_config.ap.max_connection = CONFIG_MAX_STA_CONN;
    wifi_ap_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    wifi_ap_config.ap.channel = 1;
    wifi_ap_config.ap.pmf_cfg.required = false;
}



int connect_sta()
{
    unsigned bits = device_get_state();
    if(! (bits&BIT_IS_WIFI_INIT) ){
        CHECK_AND_RET_ERR(wifi_init());
    }
    if (wifi_mode == WIFI_MODE_STA){
        esp_wifi_stop();
    } else {
        if (wifi_mode != WIFI_MODE_NULL){
            wifi_stop(); 
        }
        if(netif == NULL){
            netif = esp_netif_create_default_wifi_sta();
            if(netif == NULL) return ESP_FAIL;
        }

        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_STOP, &sta_handler, NULL));
    }

    wifi_mode = WIFI_MODE_STA;
    CHECK_AND_RET_ERR(esp_wifi_set_mode(WIFI_MODE_STA));
#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
        CHECK_AND_RET_ERR(esp_wifi_set_protocol(ESP_IF_WIFI_STA,
                                                WIFI_PROTOCOL_11B
                                                |WIFI_PROTOCOL_11G
                                                |WIFI_PROTOCOL_11N
                                                |WIFI_PROTOCOL_LR));
#endif
    const size_t ssid_len = strnlen((char*)wifi_sta_config.sta.ssid, sizeof(wifi_sta_config.sta.ssid));
    const size_t pwd_len = strnlen((char*)wifi_sta_config.sta.password, sizeof(wifi_sta_config.sta.password));
    if(ssid_len == 0 || ssid_len == sizeof(wifi_sta_config.sta.ssid) || pwd_len < MIN_WIFI_PWD_LEN){
        set_wifi_sta_config(CONFIG_WIFI_STA_DEBUG_SSID, CONFIG_WIFI_STA_DEBUG_PASSWORD);
    }
    CHECK_AND_RET_ERR(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config));
    CHECK_AND_RET_ERR(esp_wifi_start());
    return device_wait_bits(BIT_IS_STA_CONNECTION)
           &BIT_IS_STA_CONNECTION ? ESP_OK : ESP_ERR_TIMEOUT;
}


int start_ap()
{
    if(! (device_get_state()&BIT_IS_WIFI_INIT) ){
        CHECK_AND_RET_ERR(wifi_init());
    }
    if (wifi_mode != WIFI_MODE_AP){
        if (wifi_mode != WIFI_MODE_NULL){
            wifi_stop(); 
        }
        if(netif == NULL){
            netif = esp_netif_create_default_wifi_ap();
            if(netif == NULL) return ESP_FAIL;
        }
        const size_t ssid_len = strnlen((char *)wifi_ap_config.ap.ssid, sizeof(wifi_ap_config.ap.ssid));
        if(ssid_len == 0){
            set_wifi_ap_config(CONFIG_WIFI_AP_SSID, CONFIG_WIFI_AP_PASSWORD);
        }
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &ap_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &ap_handler, NULL));       
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &ap_handler, NULL));
        CHECK_AND_RET_ERR(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &ap_handler, NULL));
        wifi_mode = WIFI_MODE_AP;
        CHECK_AND_RET_ERR(esp_wifi_set_mode(WIFI_MODE_AP));
        CHECK_AND_RET_ERR(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    }
    return esp_wifi_start();
}


void wifi_stop()
{
    device_clear_state(BIT_IS_STA_CONNECTION|BIT_IS_AP_CONNECTION|BIT_IS_AP_MODE|BIT_STA_CONF_OK);
    esp_wifi_stop();
    vTaskDelay(500/portTICK_PERIOD_MS);
    if (wifi_mode == WIFI_MODE_AP){
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_START, &ap_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_AP_STOP, &ap_handler);
    } else if (wifi_mode == WIFI_MODE_STA) {
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, &sta_handler);
        esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler);
        esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_STOP, &sta_handler);
    }
    wifi_mode = WIFI_MODE_NULL;
    if(netif){
        esp_netif_destroy_default_wifi(netif);
        netif = NULL;
    }
}

