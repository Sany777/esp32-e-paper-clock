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

#define FORCE_UPDATE_COUNT_NUM 5
static Paint *paint;
static Epaper epd;

static unsigned char screen[SCREEN_WIDTH * SCREEN_HEIGHT/8];
static char text_buf[150];
static int rotate;




static sFONT* epaper_get_font(int font_num);

void epaper_init()
{
    if(!paint){
        rotate = ROTATE_0;
        paint = new Paint(screen, epd.width, epd.height);
        epd.LDirInit();
    }
}

static sFONT* epaper_get_font(int font_num)
{
    switch(font_num){
        case 48: return &Font48; 
        case 34: return &Font34; 
        case 12:return &Font12;
        case 16: return &Font16;
        case 20: return &Font20;
        case 24: return &Font24;
        case 64: return &Font64;
        default: break;
    }
    return &Font20;
}

void epaper_print_str(int hor, int ver, int font_size, color_t colored, const char *str)
{ 
    sFONT * font = epaper_get_font(font_size);
    paint->DrawStringAt(hor, ver, str, font, colored);
}

void epaper_printf(int hor, int ver, int font_size, color_t colored, const char *format, ...)
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
    epd.WaitUntilIdle();
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
        epd.WaitUntilIdle();
    }
}


void epaper_display_part()
{
    epd.DisplayPart(screen);
    epd.WaitUntilIdle();
}