#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H


#ifdef __cplusplus
extern "C" {
#endif






void set_wifi_sta_config(char *ssid, char *pwd);
int connect_sta();
int connect_ap();
void wifi_stop();
int wifi_init(void) ;




#ifdef __cplusplus
}
#endif




#endif