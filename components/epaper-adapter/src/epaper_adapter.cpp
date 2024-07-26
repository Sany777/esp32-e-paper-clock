#include "epaper_adaper.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "fonts.h"
#include "epd1in54_V2.h"

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "additional_functions.h"


// #ifdef __cplusplus
// }
// #endif


void EPaperAdapter::init()
{
  frame_ = new unsigned char[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
  text_buf = new char[MAX_SYMB];
  set_pin(EP_ON_PIN, 1);
  vTaskDelay(100);
  paint = new Paint(frame_, epd.width, epd.height);
  paint->Clear(UNCOLORED);
  epd.LDirInit();
  epd.Clear();
}

sFONT* EPaperAdapter::get_font(int font_num)
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


void EPaperAdapter::printf(int hor, int ver, int font, const char *format, ...)
{
    va_list args;
    va_start (args, format);
    vsnprintf (text_buf, MAX_SYMB, format, args);
    va_end (args);
    paint->DrawStringAt(hor, ver, text_buf, get_font(font), COLORED);
    epd.DisplayPart(frame_);
}

void EPaperAdapter::refresh()
{
    epd.DisplayFrame();
}

void EPaperAdapter::sleep()
{
  epd.Sleep();
}