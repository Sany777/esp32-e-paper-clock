#ifndef EPAPER_ADAPTER_H
#define EPAPER_ADAPTER_H

#include "stdarg.h"





#ifdef __cplusplus
extern "C" {
#endif




typedef enum {
    COLORED,
    UNCOLORED
}color_t;



void epaper_init();
void epaper_printf(int hor, int ver, int font_size, color_t colored, const char *format, ...);
void epaper_refresh();
void epaper_display();
void epaper_clear(int colored);
void draw_rect(int hor_0, int ver_0, int hor_1, int ver_1, int colored, int filled);
void draw_circle(int hor, int ver, int radius, int colored, int filled);
void draw_line(int hor_0, int ver_0, int hor_1, int ver_1, int colored);
void draw_horizontal_line(int hor_0, int hor_1, int ver, int width, int colored);
void draw_vertical_line(int ver_0, int ver_1, int hor, int width, int colored);
void epaper_set_rotate(int cur_rotate);
void epaper_print_str(int hor, int ver, int font_size, color_t colored, const char *str);

void epaper_display_part();
void epaper_display_all();



#ifdef __cplusplus
}
#endif













#endif