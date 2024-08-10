#include "device_gpio.h"

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "MPU6500.h"
#include "epaper_adapter.h"

#include "portmacro.h"
#include "esp_sleep.h"
#include "esp_log.h"


// 34 - UP, 27 - left, 35 - right, 32 - , 33 -center,


static const int joystic_pin[] = {GPIO_NUM_35,GPIO_NUM_33,GPIO_NUM_27};
static const int BUT_NUM = sizeof(joystic_pin)/sizeof(joystic_pin[0]);


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
    gpio_set_direction(GPIO_WAKEUP_PIN, GPIO_MODE_INPUT);
    gpio_set_intr_type(GPIO_WAKEUP_PIN, GPIO_INTR_POSEDGE);
}



int device_get_joystick_btn()
{
    for(int i=0; i<BUT_NUM; ++i){
        if(gpio_get_level(joystic_pin[i])){
            while(gpio_get_level(joystic_pin[i])) vTaskDelay(10);
            return i;
        }
    }
    return NO_IN_DATA;
}