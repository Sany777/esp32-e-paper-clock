#include "setting_server.h"

#include <sys/stat.h>
#include <dirent.h>
#include "cJSON.h"

#include "esp_http_server.h"
// #include "esp_ota_ops.h"
// #include "esp_partition.h"
#include "esp_chip_info.h"
#include "string.h"

#include "clock_memory.h"
#include "clock_module.h"

#include "additional_functions.h"


#define BUF_SIZE 1000

static httpd_handle_t server;
static char *server_buf;

const char *MES_DATA_NOT_READ = "Data not read";
const char *MES_DATA_TOO_LONG = "Data too long";
const char *MES_NO_MEMORY = "No memory";
const char *MES_BAD_DATA_FOMAT = "wrong data format";
const char *MES_SUCCESSFUL = "Successful";

#define SEND_REQ_ERR(req, str, label_) \
    do{ httpd_resp_send_err((req), HTTPD_400_BAD_REQUEST, (str)); goto label_;}while(0)

#define SEND_SERVER_ERR(req, str, label_) \
    do{ httpd_resp_send_err((req), HTTPD_500_INTERNAL_SERVER_ERROR, (str)); goto label_;}while(0)




static esp_err_t index_redirect_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "307 Temporary Redirect");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t get_index_handler(httpd_req_t *req)
{
    extern const unsigned char index_html_start[] asm( "_binary_index_html_start" );
    extern const unsigned char index_html_end[] asm( "_binary_index_html_end" );
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start); 
    return ESP_OK;
}

static esp_err_t get_css_handler(httpd_req_t *req)
{
    extern const unsigned char style_css_start[] asm( "_binary_style_css_start" );
    extern const unsigned char style_css_end[] asm( "_binary_style_css_end" );
    httpd_resp_set_type(req, "text/css");
    httpd_resp_send(req, (const char *)style_css_start, style_css_end - style_css_start ); 
    return ESP_OK;
}

static esp_err_t get_js_handler(httpd_req_t *req)
{
    extern const unsigned char script_js_start[] asm( "_binary_script_js_start" );
    extern const unsigned char script_js_end[] asm( "_binary_script_js_end" );
    httpd_resp_set_type(req, "text/javascript");
    httpd_resp_send(req, (const char *)script_js_start, script_js_end - script_js_start ); 
    return ESP_OK;
}

static esp_err_t get_ico_handler(httpd_req_t *req)
{
    extern const unsigned char favicon_ico_start[] asm( "_binary_favicon_ico_start" );
    extern const unsigned char favicon_ico_end[] asm( "_binary_favicon_ico_end" );
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start); 
    return ESP_OK;
}


static esp_err_t handler_close(httpd_req_t *req)
{
    httpd_resp_sendstr(req, "Goodbay!");
    vTaskDelay(100);
    stop_server();
    return ESP_OK;
}



static esp_err_t handler_set_network(httpd_req_t *req)
{
    cJSON *root, *ssid_name_j, *pwd_wifi_j;
    const char *ssid_name = NULL, *pwd_wifi = NULL;
    char *server_buf = NULL;
    int received;
    size_t pwd_len = 0, ssid_len = 0;
    const size_t total_len = req->content_len;
    clock_data_t *main_data = (clock_data_t *)req->user_ctx;
    if(total_len > BUF_SIZE){
        SEND_REQ_ERR(req, MES_DATA_TOO_LONG, label_1);
    }
    server_buf = (char *)malloc(total_len+1);
    if(server_buf == NULL){
        SEND_SERVER_ERR(req, MES_NO_MEMORY, label_1);
    }
    received = httpd_req_recv(req, server_buf, total_len);
    if (received != total_len) {
        SEND_SERVER_ERR(req, MES_DATA_NOT_READ, label_2);
    }
    server_buf[received] = 0;
    root = cJSON_Parse(server_buf);
    ssid_name_j = cJSON_GetObjectItemCaseSensitive(root, "SSID");
    pwd_wifi_j = cJSON_GetObjectItemCaseSensitive(root, "PWD");
    
    if(cJSON_IsString(ssid_name_j) && (ssid_name_j->valuestring != NULL)){
        ssid_name = ssid_name_j->valuestring;
        ssid_len = strnlen(ssid_name, MAX_STR_LEN);
    }
    if(cJSON_IsString(pwd_wifi_j) && (pwd_wifi_j->valuestring != NULL)){
        pwd_wifi = pwd_wifi_j->valuestring;
        pwd_len = strnlen(pwd_wifi, MAX_STR_LEN);
    }
    if((pwd_len == 0 && ssid_len == 0) 
        || ssid_len > MAX_STR_LEN 
        || pwd_len > MAX_STR_LEN)
    {
        SEND_REQ_ERR(req, "Data length too long", label_3);
    }
    if(ssid_len){
        memcpy(main_data->ssid, ssid_name, ssid_len+1);
        memory_write(main_data, DATA_SSID);
    }
    if(pwd_len){
        memcpy(main_data->pwd, pwd_wifi, pwd_len+1);
        memory_write(main_data, DATA_PWD);
    }
    free(server_buf);
    cJSON_Delete(root);
    httpd_resp_sendstr(req, MES_SUCCESSFUL);
    return ESP_OK;
label_3: 
    cJSON_Delete(root);
label_2:
    free(server_buf);
label_1:
    return ESP_FAIL;
}

static esp_err_t handler_set_openweather_data(httpd_req_t *req)
{
    const char *key = NULL, *city_name = NULL;
    cJSON *root, *city_j, *key_j;
    const char *ssid_name = NULL, *pwd_wifi = NULL;
    char *server_buf = NULL;
    int received;
    size_t key_len = 0, city_len = 0;
    const int total_len = req->content_len;
    clock_data_t *main_data = (clock_data_t *)req->user_ctx;
    if(total_len > BUF_SIZE){
        SEND_REQ_ERR(req, MES_DATA_TOO_LONG, label_1);
    }
    server_buf = (char *) malloc(total_len+1);
    if(server_buf == NULL){
        SEND_SERVER_ERR(req, MES_NO_MEMORY, label_1);
    }
    received = httpd_req_recv(req, server_buf, total_len);
    if (received != total_len) {
        SEND_SERVER_ERR(req, MES_DATA_NOT_READ, label_2);
    }
    server_buf[received] = 0;
    root = cJSON_Parse(server_buf);
    city_j = cJSON_GetObjectItemCaseSensitive(root, "City");
    key_j = cJSON_GetObjectItemCaseSensitive(root, "Key");
    if(cJSON_IsString(city_j) && (city_j->valuestring != NULL)){
        city_name = city_j->valuestring;
        city_len = strnlen(city_name, MAX_STR_LEN);
    }
    if(cJSON_IsString(key_j) && (key_j->valuestring != NULL)){
        key = key_j->valuestring;
        key_len = strnlen(key, MAX_STR_LEN);
    }
    if((city_len == 0 && key_len == 0) 
        || city_len > MAX_STR_LEN)
    {
        SEND_REQ_ERR(req, MES_BAD_DATA_FOMAT, label_3);
    }
    if(key_len == API_LEN){
        memcpy(main_data->api_key, key, key_len+1);
        memory_write(main_data, DATA_API);
    }
    if(city_len){
        memcpy(main_data->city_name, city_name, city_len+1); 
        memory_write(main_data, DATA_CITY);
    }
    cJSON_Delete(root);
    free(server_buf);
    httpd_resp_sendstr(req, MES_SUCCESSFUL);
    return ESP_OK;
label_3:
    cJSON_Delete(root);
label_2:
    free(server_buf);
label_1:
    return ESP_FAIL;
}

const char *get_chip(int model_id)
{
    switch(model_id){
        case 1: return "ESP32";
        case 2: return "ESP32-S2";
        case 3: return "ESP32-S3";
        case 5: return "ESP32-C3";
        case 6: return "ESP32-H2";
        case 12: return "ESP32-C2";
        default: break;
    }
    return "uknown";
}

static esp_err_t handler_get_info(httpd_req_t *req)
{
    const char *sys_info;
    cJSON *root = cJSON_CreateObject();
    if(root == NULL){
        SEND_SERVER_ERR(req, MES_NO_MEMORY, err);
    }
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    cJSON_AddStringToObject(root, "version", IDF_VER);
    cJSON_AddStringToObject(root, "chip", get_chip(chip_info.model));
    cJSON_AddNumberToObject(root, "revision", chip_info.revision);
    sys_info = cJSON_Print(root);
    if(sys_info){
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, sys_info);
        free((void *)sys_info);
    }
    cJSON_Delete(root);
    return ESP_OK;
err:
    return ESP_FAIL;
}

static esp_err_t handler_set_time(httpd_req_t *req)
{
    long long time;
    int received;
    const int total_len = req->content_len;
    char * const server_buf = (char *)req->user_ctx;
    if(total_len > BUF_SIZE){
        SEND_REQ_ERR(req, MES_DATA_TOO_LONG, err);
    }
    received = httpd_req_recv(req, server_buf, total_len);
    if (received != total_len) {
        SEND_REQ_ERR(req, MES_DATA_NOT_READ, err);
    }
    server_buf[total_len] = 0;
    time = atoll(server_buf);
    if(!time){
        SEND_REQ_ERR(req, MES_BAD_DATA_FOMAT, err);
    }
    set_time_ms(time);
    httpd_resp_sendstr(req, "success");
    return ESP_OK;
err:
    return ESP_FAIL;
}


static esp_err_t handler_give_data(httpd_req_t *req)
{
    const char *message = "success";
//     clock_data_t * main_data = (clock_data_t *)req->user_ctx;
//     char *notif_send = malloc(LEN_DATA_// SEND_NOTIF);
//     if(notif_send == NULL){
//         // SEND_REQ_ERR(req, "Not enough storage", err);
//     }
//     EventBits_t uxBits = xEventGroupGetBits(dwin_event_group);
//     httpd_resp_set_type(req, "application/json");
//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddStringToObject(root, "SSID", name_SSID);
//     cJSON_AddStringToObject(root, "PWD", pwd_WIFI);
//     cJSON_AddStringToObject(root, "Key", main_data->api_key);
//     cJSON_AddStringToObject(root, "City", main_data->city_name);
//     cJSON_AddNumberToObject(root, "Status", uxBits);
//     for(uint8_t dayi=0, notif=0, i_s=0, hour=0, min=0; ; dayi++){
//         if(dayi >= SIZE_WEEK){
//             notif++;
//             if(notif == NOTIF_PER_DAY) break;
//             dayi = 0;
//         }
//         hour = VALUE_NOTIF_HOUR(notif, dayi);
//         min = VALUE_NOTIF_MIN(notif, dayi);
//         notif_send[i_s++] = hour/10 +'0';
//         notif_send[i_s++] = hour%10 +'0';
//         notif_send[i_s++] = min/10 +'0';
//         notif_send[i_s++] = min%10 + '0';
//     }
//     notif_send[LEN_DATA_// SEND_NOTIF-1] = 0;
//     cJSON_AddStringToObject(root, "Notification", notif_send);
//     const char *data_info = cJSON_Print(root);
//     if(!data_info){
//         free(notif_send);
//         // SEND_REQ_ERR(req, "Not enough storage", err);
//     }
//     httpd_resp_sendstr(req, data_info);
//     free((void *)data_info);
//     free(notif_send);
//     cJSON_Delete(root);
//     return ESP_OK;
// err:
    return ESP_FAIL;
}

static esp_err_t handler_set_flag(httpd_req_t *req)
{
//     const int total_len = req->content_len;
//     char * const server_buf = (char *)req->user_ctx;
//     if(total_len > BUF_SIZE){
//         // SEND_REQ_ERR(req, "Content too long", err);
//     }
//     const int received = httpd_req_recv(req, server_buf, total_len);
//     if (received != total_len) {
//         // SEND_REQ_ERR(req, "Data not read", err);
//     }
//     server_buf[total_len] = 0;
//     long flag = atoll(server_buf);
//     for(int i=0; i<NUMBER_STORED_FLAGS; i++){
//         if(flag&(1<<i)){
//             xEventGroupSetBits(dwin_event_group, (1<<i));
//         }else {
//             xEventGroupClearBits(dwin_event_group, (1<<i));
//         }
//     }
//     memory_write(NULL, DATA_FLAGS);
//     httpd_resp_sendstr(req, "Set flags successfully");
//     return ESP_OK;
// err:
    return ESP_FAIL;
}


static esp_err_t set_notif_handler(httpd_req_t *req)
{
//     const int total_len = req->content_len;
//     if(total_len != LEN_DATA_// SEND_NOTIF-1){
//         // SEND_REQ_ERR(req, "Wrong data format", err);
//     }
//     char * const server_buf = malloc(LEN_DATA_// SEND_NOTIF);
//     if(server_buf == NULL){
//         // SEND_REQ_ERR(req, "Not enough storage", err);
//     }
//     clock_data_t * main_data = (clock_data_t *)req->user_ctx;
//     const int received = httpd_req_recv(req, server_buf, total_len);
//     if (received != total_len) {
//         // SEND_REQ_ERR(req, "Data not read", _err);
//     }
//     for(size_t  i=0,num_notif = 0,dayi=0,val=0; i<total_len; dayi++){
//         if(dayi >= SIZE_WEEK){
//             num_notif++;
//             if(num_notif == NOTIF_PER_DAY) break;
//             dayi = 0;
//         }
//         val = GET_NUMBER(server_buf[i])*10 + GET_NUMBER(server_buf[i+1]);
//         if(!IS_HOUR(val)){
//            // SEND_REQ_ERR(req, "Value hour is wrong", _err); 
//         }
//         SET_NOTIF_HOUR(num_notif, dayi, val);
//         val = GET_NUMBER(server_buf[i+2])*10 + GET_NUMBER(server_buf[i+3]);
//         if(!IS_MIN_OR_SEC(val)){
//            // SEND_REQ_ERR(req, "Value minute is wrong", _err); 
//         }
//         SET_NOTIF_MIN(num_notif, dayi, val);
//         i+=4;
//     }
//     memory_write(main_data, DATA_NOTIF);
//     httpd_resp_sendstr(req, "Update notification");
//     free(server_buf);
//     return ESP_OK;
// _err:
//     free(server_buf);
// err:
    return ESP_FAIL;
}


int stop_server()
{
    esp_err_t err = ESP_FAIL;
    if(server_buf){
        free(server_buf); 
        server_buf = NULL;
    }
    if(server){
        err = httpd_stop(server);
        server = NULL;
    }
    return err;
}


int start_server(clock_data_t *main_data)
{
    if(main_data == NULL || server != NULL) return ESP_FAIL;
    if(server_buf == NULL){
       server_buf = (char *)malloc(BUF_SIZE); 
       if(!server_buf) return ESP_ERR_NO_MEM;
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 29;
    config.uri_match_fn = httpd_uri_match_wildcard;
    CHECK_AND_RET_ERR(httpd_start(&server, &config));

    // httpd_uri_t // SEND_flags = {
    //     .uri      = "/Status",
    //     .method   = HTTP_POST,
    //     .handler  = handler_set_flag,
    //     .user_ctx = server_buf
    // };
    
    // httpd_register_uri_handler(server, &// SEND_flags);

     
    httpd_uri_t get_info = {
        .uri      = "/getinfo",
        .method   = HTTP_GET,
        .handler  = handler_get_info,
        .user_ctx = server_buf
    };
    httpd_register_uri_handler(server, &get_info);

    httpd_uri_t get_setting = {
        .uri      = "/data?",
        .method   = HTTP_GET,
        .handler  = handler_give_data,
        .user_ctx = main_data
    };
    httpd_register_uri_handler(server, &get_setting);
    
     httpd_uri_t close_uri = {
        .uri      = "/close",
        .method   = HTTP_POST,
        .handler  = handler_close,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &close_uri);

     httpd_uri_t net_uri = {
        .uri      = "/Network",
        .method   = HTTP_POST,
        .handler  = handler_set_network,
        .user_ctx = main_data
    };
    httpd_register_uri_handler(server, &net_uri);

     httpd_uri_t api_uri = {
        .uri      = "/API",
        .method   = HTTP_POST,
        .handler  = handler_set_openweather_data,
        .user_ctx = main_data
    };
    httpd_register_uri_handler(server, &api_uri);

     httpd_uri_t time_uri = {
        .uri      = "/time",
        .method   = HTTP_POST,
        .handler  = handler_set_time,
        .user_ctx = server_buf
    };
    httpd_register_uri_handler(server, &time_uri);

     httpd_uri_t index_uri = {
        .uri      = "/index.html",
        .method   = HTTP_GET,
        .handler  = get_index_handler ,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &index_uri);


     httpd_uri_t get_ico = {
        .uri      = "/favicon.ico",
        .method   = HTTP_GET,
        .handler  = get_ico_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &get_ico);

     httpd_uri_t get_style = {
        .uri      = "/style.css",
        .method   = HTTP_GET,
        .handler  = get_css_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &get_style);

     httpd_uri_t get_script = {
        .uri      = "/script.js",
        .method   = HTTP_GET,
        .handler  = get_js_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &get_script);

     httpd_uri_t notif_uri = {
        .uri      = "/Notification",
        .method   = HTTP_POST,
        .handler  = set_notif_handler,
        .user_ctx = main_data
    };
    httpd_register_uri_handler(server, &notif_uri);

     httpd_uri_t redir_uri = {
        .uri      = "/*",
        .method   = HTTP_GET,
        .handler  = index_redirect_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &redir_uri);

    return ESP_OK;
}