#include "epaper_adapter.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "epaper.h"
#include "epdpaint.h"
#include "fonts.h"
#include "epd1in54_V2.h"


#include "cstring"

static int SCREEN_WIDTH   = 200;
static int SCREEN_HEIGHT  = 200;
static int MAX_SYMB       = 100;
static int rotate;
static Paint *paint;
static Epd epd;
unsigned char *screen;
char *text_buf;


static sFONT* epaper_get_font(int font_num);



void epaper_init()
{
    if(!screen){
        screen = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
    }
    if(!text_buf){
        text_buf = new char[MAX_SYMB];
    }
    if(!paint){
        paint = new Paint(screen, epd.width, epd.height);
        paint->SetRotate(rotate);
        paint->Clear(UNCOLORED);
        epd.LDirInit();
        epd.Clear();
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


void epaper_printf(int hor, int ver, int font, const char *format, ...)
{
        va_list args;
        va_start (args, format);
        vsnprintf (text_buf, MAX_SYMB, format, args);
        va_end (args);
        int horiz = strlen(text_buf)*font/4;
        paint->DrawFilledRectangle(hor, ver, horiz+hor, ver+font, UNCOLORED);
        paint->DrawStringAt(hor, ver, text_buf, epaper_get_font(font), COLORED);
        epd.DisplayPart(screen);
}

void epaper_refresh()
{
    epd.DisplayFrame();
}


void epaper_clear()
{
    paint->Clear(UNCOLORED);
    epd.Clear();
}

void epaper_sleep()
{
    epd.Sleep();
}

void epaper_set_rotate(int cur_rotate)
{
    if(rotate != cur_rotate){
        rotate = cur_rotate;
        paint->SetRotate(rotate);
        paint->Clear(UNCOLORED);
        epd.Clear();
        epd.DisplayPart(screen);
    }
}