#include "epaper_adapter.h"

#include "epaper.h"
#include "epdpaint.h"
#include "fonts.h"
#include "epd1in54_V2.h"

#include "cstring"
#include "cstdio"


#define SCREEN_WIDTH    200
#define SCREEN_HEIGHT   200
#define MAX_SYMB        100
#define FORCE_UPDATE_COUNT_NUM 10
static Paint *paint;
static Epd epd;
#define SCREEN_BUF_SIZE  (SCREEN_WIDTH * SCREEN_HEIGHT / 8)
bool is_rotate;

static unsigned char screen[SCREEN_BUF_SIZE];
static char text_buf[MAX_SYMB];
static int rotate;
static int el_color, back_color;

static sFONT* epaper_get_font(int font_num);

void epaper_init()
{
    if(!paint){
        rotate = ROTATE_0;
        paint = new Paint(screen, epd.width, epd.height);
        back_color = UNCOLORED;
        paint->SetRotate(rotate);
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


void epaper_printf(int hor, int ver, int font_size, int colored, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (text_buf, MAX_SYMB, format, args);
    va_end (args);
    paint->DrawStringAt(hor, ver, text_buf, epaper_get_font(font_size), colored);
}

void epaper_refresh()
{
    epd.DisplayFrame();
}

void epaper_display()
{
    static int update_count;
    epd.WaitUntilIdle();
    if(is_rotate || update_count>FORCE_UPDATE_COUNT_NUM){
        epaper_display_all();
        update_count = 0;
    } else {
        epaper_display_part();
        update_count += 1;
    }
}

void epaper_display_all()
{
    epd.Clear();
    epd.WaitUntilIdle();
    epd.Display(screen);
    epd.WaitUntilIdle();
    is_rotate = false;
}

void epaper_clear(int colored)
{
    back_color = colored;
    memset(screen, colored ? 0xff : 0, sizeof(screen));
    epd.Clear();
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

void epaper_set_rotate(int cur_rotate)
{
    if(rotate != cur_rotate){
        paint->Clear(back_color);
        rotate = cur_rotate;
        paint->SetRotate(rotate);
        int new_width = paint->GetHeight();
        int new_height = paint->GetWidth();
        paint->SetHeight(new_height);
        paint->SetWidth(new_width);
        is_rotate = true;
        epd.WaitUntilIdle();
    }
}


void epaper_display_part()
{
    epd.DisplayPart(screen);
    epd.WaitUntilIdle();
}