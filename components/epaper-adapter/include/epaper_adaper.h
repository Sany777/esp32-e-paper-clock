#ifndef EPAPER_ADAPTER_H
#define EPAPER_ADAPTER_H

#include "stdarg.h"
#include "stdint.h"


#include "epaper.h"
#include "epdpaint.h"



#define EP_ON_PIN 22
#define COLORED     0
#define UNCOLORED   1



class EPaperAdapter{
private:

    static constexpr int SCREEN_WIDTH = 200;
    static constexpr int SCREEN_HEIGHT = 200;
    static constexpr int MAX_SYMB = 100;
    int rotate;
    Paint *paint;
    Epd epd;
    unsigned char *screen;
    char *text_buf;
    sFONT* get_font(int font_num);
    
public:
    void refresh();
    void init();
    void printf(int ver, int hor, int font, const char * format, ...);
    void sleep();
    void set_rotate(int cur_rotate);
};


















#endif