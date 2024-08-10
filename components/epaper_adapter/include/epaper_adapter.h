#ifndef EPAPER_ADAPTER_H
#define EPAPER_ADAPTER_H

#include "stdarg.h"





#ifdef __cplusplus
extern "C" {
#endif





#define COLORED     0
#define UNCOLORED   1


void epaper_init();
void epaper_printf(int hor, int ver, int font_size, int colored, const char *format, ...);
void epaper_refresh();
void epaper_display();
void epaper_display_all();
void epaper_clear(int colored);
void draw_rect(int hor_0, int ver_0, int hor_1, int ver_1, int colored, int filled);
void draw_circle(int hor, int ver, int radius, int colored, int filled);
void draw_line(int hor_0, int ver_0, int hor_1, int ver_1, int colored);
void draw_horizontal_line(int hor_0, int hor_1, int ver, int width, int colored);
void draw_vertical_line(int ver_0, int ver_1, int hor, int width, int colored);
void epaper_set_rotate(int cur_rotate);
void epaper_display_part();


#ifdef __cplusplus
}
#endif













#endif