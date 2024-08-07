// #include "http_client.h"

// #include "esp_http_client.h"


// static const char *buy_key = "\"buy\":\"";
// static  const char *sale_key = "\"sale\":\"";


// static char ** get_pos_values(char *buf, const size_t buf_len, const char *key)
// {
//     if(buf == NULL) return NULL;
//     size_t keys_list_len = 0, key_numb = 0;
//     char **pos_list = NULL;
//     const size_t key_size = strlen(key);
//     const char *END = buf+buf_len;
//     while(buf = strstr(buf, key), buf && END > buf) {
//         if(key_numb >= keys_list_len){
//             size_t new_keys_list_len = key_numb+RESIZE_KEYS_STEP;
//             char **new_list = (char**) realloc(pos_list, new_keys_list_len *sizeof(char*));
//             if(!new_list)break;
//             if(keys_list_len){
//                 for(int i=key_numb; i<new_keys_list_len; i++){
//                     new_list[i] = NULL;
//                 }
//             }
//             pos_list = new_list;
//             keys_list_len = new_keys_list_len;
//         }
//         /*step ahead*/
//         buf += key_size;
//         pos_list[key_numb++] = buf; 
//     }
//     return pos_list;
// }

// static esp_err_t http_event_handler(esp_http_client_event_t *evt)
// {
//     static char *output_buffer; 
//     static int output_len;      
//     switch(evt->event_id) {
//     case HTTP_EVENT_ON_DATA:
//         if(!esp_http_client_is_chunked_response(evt->client)){
//             int copy_len = 0;
//             if(evt->user_data){
//                 copy_len = MIN(evt->data_len, (CLIENT_BUF_LEN - output_len));
//                 if (copy_len) {
//                     memcpy(evt->user_data + output_len, evt->data, copy_len);
//                 }
//             } else {
//                 const int buffer_len = esp_http_client_get_content_length(evt->client);
//                 if (output_buffer == NULL) {
//                     output_buffer = (char *) malloc(buffer_len);
//                     output_len = 0;
//                     if (output_buffer == NULL) {
//                         return ESP_FAIL;
//                     }
//                 }
//                 copy_len = MIN(evt->data_len, (buffer_len - output_len));
//                 if (copy_len) {
//                     memcpy(output_buffer + output_len, evt->data, copy_len);
//                 }
//             }
//             output_len += copy_len;
//         }
//         break;
//     case HTTP_EVENT_ON_FINISH:
//         if(output_buffer != NULL){
//             free(output_buffer);
//             output_buffer = NULL;
//         }
//         output_len = 0;
//         break;
//     case HTTP_EVENT_DISCONNECTED:
//         if (output_buffer != NULL){
//             free(output_buffer);
//             output_buffer = NULL;
//         }
//         output_len = 0;
//         break;
//     default:break;
//     }
//     return ESP_OK;
// }

// static void split_words(char *buf, size_t data_len)
// {
//     const char *end = buf + data_len;
//     char c;
//     while(buf != end){
//         c = *(buf);
//         if(c == '}' || c == ',' || c == '"'){
//             *(buf) = 0;
//         }
//         ++buf;
//     }
// }

// void get_weather()
// {
//     // EventBits_t xEventGroup = xEventGroupWaitBits( dwin_event_group, 
//     //                                     BIT_PROCESS,   
//     //                                     false, false, 
//     //                                     WAIT_PROCEES);  
//     // weather_PIC = NO_WEATHER_PIC;
//     // xEventGroupClearBits(
//     //     dwin_event_group,BIT_WEATHER_OK
//     //     |BIT_RESPONSE_400_SERVER);   


//     DWIN_IF_FALSE_GOTO(!(xEventGroup&BIT_DENIED_STA), st_1);

//     if(strnlen(api_KEY, SIZE_BUF) != SIZE_API){
//         if(!(xEventGroup&BIT_WRONG_API_KEY)){
//             xEventGroupSetBits(xEventGroup, BIT_WRONG_API_KEY);
//         }
//         goto st_1;
//     } else if(xEventGroup&BIT_WRONG_API_KEY){
//         xEventGroupClearBits(xEventGroup, BIT_WRONG_API_KEY);
//     }
//     if(strnlen(name_CITY, SIZE_BUF) == 0){
//         goto st_1;
//     }
//     if(!(xEventGroup&BIT_CON_STA_OK)){
//         set_new_command(START_STA);
//         vTaskDelay(100);
//         xEventGroup = xEventGroupWaitBits(dwin_event_group, 
//                                             BIT_PROCESS,   
//                                             false, false, 
//                                             WAIT_PROCEES); 
//     }
//     DWIN_IF_FALSE_GOTO(xEventGroup&BIT_CON_STA_OK, st_1);
//     char *url_buf = (char*)calloc(1, SIZE_URL_BUF);
//     DWIN_CHECK_NULL_AND_GO(url_buf, "", st_1);
//     char *local_response_buffer = (char*)malloc(CLIENT_BUF_LEN);
//     DWIN_CHECK_NULL_AND_GO(local_response_buffer, "", st_2);
//     snprintf(url_buf, SIZE_URL_BUF, "%s%s%s%s", FIRST_URL, name_CITY, SECOND_URL, api_KEY);
//     esp_http_client_config_t config = {
//         .url = url_buf,
//         .event_handler = http_event_handler,
//         .user_data = (void*)local_response_buffer,    
//         .method = HTTP_METHOD_GET,
//         .buffer_size = CLIENT_BUF_LEN,
//         .auth_type = HTTP_AUTH_TYPE_NONE
//     };
//     esp_http_client_handle_t client = esp_http_client_init(&config);
//     DWIN_CHECK_AND_GO(esp_http_client_perform(client), st_3);
//     const size_t data_len = esp_http_client_get_content_length(client);
//     DWIN_IF_FALSE_GOTO(data_len != 0, st_3);
//     char **pop = get_pos_values(local_response_buffer, data_len, "\"pop\":");
//     char **temp = get_pos_values(local_response_buffer, data_len, "\"temp\":");
//     char **temp_feel = get_pos_values(local_response_buffer, data_len, "\"feels_like\":");
//     char **description = get_pos_values(local_response_buffer, data_len, "\"description\":\"");
//     char **id = get_pos_values(local_response_buffer, data_len, "\"id\":");
//     char **sunrise = get_pos_values(local_response_buffer, data_len, "\"sunrise\":");
//     char **sunset = get_pos_values(local_response_buffer, data_len, "\"sunset\":");
//     char **dt_txt = get_pos_values(local_response_buffer, data_len, "\"dt_txt\":");
//     char **pod = get_pos_values(local_response_buffer, data_len, "\"pod\":\"");
//     if( !(pop 
//             && temp
//             && temp_feel
//             && description
//             && id
//             && sunrise
//             && sunset
//             && dt_txt
//             && pod) )
//     {
//         xEventGroupSetBits(dwin_event_group, BIT_RESPONSE_400_SERVER); 
//         goto st_4;
//     }
//     xEventGroupSetBits(dwin_event_group, BIT_WEATHER_OK);
//     split_words(local_response_buffer, data_len);
//     struct tm timeinfo;
//     time_t time_now = atol(sunrise[0]);
//     localtime_r(&time_now, &timeinfo);
//     sunrise_HOUR = timeinfo.tm_hour;
//     sunrise_MIN = timeinfo.tm_min;
//     time_now = atol(sunset[0]);
//     localtime_r(&time_now, &timeinfo);
//     sunset_HOUR = timeinfo.tm_hour;
//     sunset_MIN = timeinfo.tm_min;
//     dt_TX = atoi((dt_txt[0]+SHIFT_DT_TX));
//     weather_PIC = get_pic(atoi(id[0]), pod[0][0] == 'n');
//     strncpy(description_WEATHER, description[0], MAX_LEN_DESCRIPTION);
//     for(int i=0; pop[i] && temp_feel[i] && i < NUMBER_DATA_WEATHER; i++){
//         temp_FEELS_LIKE[i] = atof(temp_feel[i]);
//         PoP[i] = atof(pop[i])*100;
//     }
//     temp_OUTDOOR = atof(temp[0]);
// st_4:
//     if(dt_txt)free(dt_txt);
//     if(pop)free(pop);
//     if(sunrise)free(sunrise);
//     if(sunset)free(sunset);
//     if(id)free(id);
//     if(temp)free(temp);
//     if(description)free(description);
//     if(temp_feel)free(temp_feel);
//     if(pod)free(pod);
// st_3:
//     esp_http_client_cleanup(client);
//     free(local_response_buffer);
// st_2:
//     free(url_buf);
// st_1:
//     set_new_command(UPDATE_WEATHER_COMPLETE);
// }


// static void set_currency_state(dwin_data_t *main_data, float newUsd, float newEur)
// {
//     if(usd_Sale != DWIN_NO_DATA){
//         if(newUsd > usd_Sale){
//             usd_State = GO_UP;
//         } else if(newUsd < usd_Sale){
//             usd_State = GO_DOWN;
//         } else {
//             usd_State = NO_CHANGE;
//         }
//     }
//     if(eur_Sale != DWIN_NO_DATA){
//         if(newEur > eur_Sale){
//             eur_State = GO_UP;
//         } else if(newEur < eur_Sale){
//             eur_State = GO_DOWN;
//         } else {
//             eur_State = NO_CHANGE;
//         }
//     }
// }


// void get_currency(dwin_data_t *main_data)
// {
//     EventBits_t xEventGroup = xEventGroupWaitBits( dwin_event_group, 
//                                         BIT_PROCESS,   
//                                         false, false, 
//                                         WAIT_PROCEES);
                                                       
//     DWIN_IF_FALSE_GOTO(xEventGroup&BIT_CON_STA_OK, _end);
//     char url_buf[] = SIMPLE_PRIVAT_API;
//     char *local_response_buffer = (char*)calloc(1, CLIENT_BUF_LEN);
//     DWIN_CHECK_NULL_AND_GO(local_response_buffer, "", _end);
//     esp_http_client_config_t config = {
//         .url = SIMPLE_PRIVAT_API,
//         .event_handler = http_event_handler,
//         .user_data = (void*)local_response_buffer,    
//         .method = HTTP_METHOD_GET,
//         .buffer_size = CLIENT_BUF_LEN,
//         .auth_type = HTTP_AUTH_TYPE_NONE
//     };

//     esp_http_client_handle_t client = esp_http_client_init(&config);

//     if(esp_http_client_perform(client) == ESP_OK){
       
//         const size_t data_len = esp_http_client_get_content_length(client);
//         if(data_len){
//             char *usd_pos, *eur_pos, **usd_bay, **usd_sale, **eur_bay, **eur_sale;
//             usd_pos = strstr(local_response_buffer, "\"USD\"");
//             eur_pos = strstr(local_response_buffer, "\"EUR\"");
//             if(usd_pos && eur_pos){

//                 usd_bay = get_pos_values(usd_pos, data_len, buy_key);
//                 usd_sale = get_pos_values(usd_pos, data_len, sale_key);
//                 eur_bay = get_pos_values(eur_pos, data_len, buy_key);
//                 eur_sale = get_pos_values(eur_pos, data_len, sale_key);
//                 split_words(local_response_buffer, data_len);
                
//                 if(usd_bay && usd_sale && eur_bay && eur_sale){
//                     float new_usd_bay  = atof(usd_bay[0]);
//                     float new_usd_sale = atof(usd_sale[0]);
//                     float new_eur_bay  = atof(eur_bay[0]);
//                     float new_eur_sale = atof(eur_sale[0]);
//                     set_currency_state(main_data, new_usd_sale, new_eur_sale);
//                     usd_Bay = new_usd_bay;
//                     usd_Sale = new_usd_sale;
//                     eur_Bay = new_eur_bay;
//                     eur_Sale = new_eur_sale;
//                 }
//                 if(usd_bay)free(usd_bay);
//                 if(usd_sale)free(usd_sale);
//                 if(eur_bay)free(eur_bay);
//                 if(eur_sale)free(eur_sale);
//             }
//         }
//     }

//     esp_http_client_cleanup(client);
//     free(local_response_buffer);
//     set_new_command(UPDATE_DATA_COMPLETE);
//     return;
// _end:
//     init_currency_val(main_data);
// }
