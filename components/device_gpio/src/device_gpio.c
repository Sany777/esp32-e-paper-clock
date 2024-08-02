#include "device_gpio.h"

#include "driver/gpio.h"


#define BUT_NUM 5

static int joystic_pin[BUT_NUM] = {GPIO_NUM_34,GPIO_NUM_35,GPIO_NUM_32,GPIO_NUM_33,GPIO_NUM_27};

int device_set_pin(int pin, unsigned state)
{
    gpio_set_direction((gpio_num_t )pin, GPIO_MODE_INPUT_OUTPUT);
    return gpio_set_level((gpio_num_t )pin, state);
}


void device_gpio_init()
{
    for(int i=0; i<BUT_NUM; ++i){
        gpio_set_direction(joystic_pin[i], GPIO_MODE_INPUT);
        gpio_pulldown_en(joystic_pin[i]);
    }
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_0);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}


int device_get_joystick_btn()
{
    for(int i=0; i<BUT_NUM; ++i){
        if(gpio_get_level(joystic_pin[i])){
            return i;
        }
    }
    return -1;
}