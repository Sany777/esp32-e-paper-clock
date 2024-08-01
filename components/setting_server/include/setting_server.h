#ifndef SETTING_SERVER_H
#define SETTING_SERVER_H



#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

int stop_server();
int start_server();
extern bool server_run, is_connect;



#ifdef __cplusplus
}
#endif


#endif