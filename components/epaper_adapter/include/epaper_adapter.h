#ifndef EPAPER_ADAPTER_H
#define EPAPER_ADAPTER_H

#include "stdarg.h"





#ifdef __cplusplus
extern "C" {
#endif





#define COLORED     0
#define UNCOLORED   1



void epaper_refresh();
void epaper_init();
void epaper_printf(int ver, int hor, int font, const char * format, ...);
void epaper_sleep();
void epaper_set_rotate(int cur_rotate);


#ifdef __cplusplus
}
#endif













#endif