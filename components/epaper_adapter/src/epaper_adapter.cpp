#include "epaper_adapter.h"

#include "epaper.h"
#include "epdpaint.h"
#include "fonts.h"
#include "epd1in54_V2.h"

#include "cstring"
#include "cstdio"

#include "esp_log.h"

#define SCREEN_HEIGHT   200
#define SCREEN_WIDTH    200

static Paint *paint;
static Epaper epd;

static unsigned char screen[SCREEN_WIDTH * SCREEN_HEIGHT/8];

static char text_buf[150];
static const int USE_SCREEN_WIDTH = SCREEN_WIDTH-10;
static sFONT *font_list[] = {&Font12, &Font16, &Font20, &Font24, &Font34, &Font48, &Font64};



void epaper_init()
{
    if(!paint){
        paint = new Paint(screen, epd.width, epd.height);
    }
    epd.LDirInit();
}


static sFONT* get_font(size_t font_id)
{
    if(font_id >= FONT_SIZE_MAX){
        return NULL;
    }
    return font_list[font_id];
}

void epaper_print_str(int hor, int ver, font_size_t font_size, color_t colored, const char *str)
{
    sFONT * font = get_font(font_size);
    if(font){
        paint->DrawStringAt(hor, ver, str, font, colored);
    }
}

static sFONT* get_fit_font(font_size_t font_id, size_t char_num)
{
    size_t ind = font_id;
    sFONT * font = NULL;
    size_t str_width = 0;
    do{
        if(ind >= FONT_SIZE_MAX)break;
        font = get_font(ind);
        ind -= 1;
        str_width = font->Width * char_num;
    }while(str_width>USE_SCREEN_WIDTH);
    return font;
}

void epaper_print_centered_str(int ver, font_size_t font_size, color_t colored, const char *str)
{
    int h = 5, str_width;
    const size_t str_len = strlen(str);
    sFONT * font = get_fit_font(font_size, str_len);
    if(font){
        str_width = font->Width * str_len;
        if(str_width < USE_SCREEN_WIDTH){
            h += (USE_SCREEN_WIDTH - str_width) / 2;  
        }
        paint->DrawStringAt(h, ver, str, font, colored);  
    }
}

void epaper_printf_centered(int ver, font_size_t font_size, color_t colored, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (text_buf, sizeof(text_buf), format, args);
    va_end (args);
    epaper_print_centered_str(ver, font_size, colored, text_buf);
}

void epaper_printf(int hor, int ver, font_size_t font_size, color_t colored, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (text_buf, sizeof(text_buf), format, args);
    va_end (args);
    epaper_print_str(hor, ver, font_size, colored, text_buf);
}

void epaper_refresh()
{
    epd.DisplayFrame();
}

void epaper_display_all()
{
    epd.WaitUntilIdle();
    epd.Display(screen);
}

void epaper_clear(int colored)
{
    memset(screen, colored ? 0xff : 0, sizeof(screen));
}


void draw_rect(int hor_0, int ver_0, int hor_1, int ver_1, int colored, int filled)
{
    if(filled){
        paint->DrawFilledRectangle(hor_0, ver_0, hor_1, ver_1, colored);
    } else {
        paint->DrawRectangle(hor_0, ver_0, hor_1, ver_1, colored);
    }
}

void draw_circle(int hor, int ver, int radius, int colored, int filled)
{
    if(filled){
        paint->DrawFilledCircle(hor, ver, radius, colored);
    } else {
        paint->DrawCircle(hor, ver, radius, colored);
    }
}

void draw_line(int hor_0, int ver_0, int hor_1, int ver_1, int colored)
{
    paint->DrawLine(hor_0, ver_0, hor_1, ver_1, colored);
}

void draw_horizontal_line(int hor_0, int hor_1, int ver, int width, int colored)
{
    paint->DrawFilledRectangle(hor_0, ver, hor_1, ver+width, colored);
}

void draw_vertical_line(int ver_0, int ver_1, int hor, int width, int colored)
{
    paint->DrawFilledRectangle(hor, ver_0, hor+width, ver_1, colored);
}


void epaper_set_rotate(int rotate)
{
    if(rotate >= 0 && rotate < 4){
        paint->SetRotate(rotate);       
    }
}


void epaper_display_part()
{
    epd.WaitUntilIdle();
    epd.DisplayPart(screen);
}


void epaper_wait()
{
    epd.WaitUntilIdle();
}