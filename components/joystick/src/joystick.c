#include "joystick.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "additional_functions.h"

#define BUT_NUM 5
static int joystic_pin[BUT_NUM] = {34,35,32,33,27};


void joystick_init()
{
    for(int i=0; i<BUT_NUM; ++i){
        gpio_set_direction(joystic_pin[i], GPIO_MODE_INPUT);
        gpio_pulldown_en(joystic_pin[i]);
    }
}

int get_joystick_btn()
{
    for(int i=0; i<BUT_NUM; ++i){
        if(gpio_get_level(joystic_pin[i])){
            return i;
        }
    }
    return -1;
}