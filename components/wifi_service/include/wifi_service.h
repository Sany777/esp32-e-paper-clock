#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif






void set_wifi_sta_config(char *ssid, char *pwd);
int connect_sta();
int start_ap();
int wifi_init(void) ;
void wifi_stop();
void wifi_off();



#ifdef __cplusplus
}
#endif




#endif