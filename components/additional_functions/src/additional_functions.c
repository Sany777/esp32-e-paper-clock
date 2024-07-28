#include "additional_functions.h"

#include "portmacro.h"
#include "esp_sleep.h"
#include "driver/gpio.h"





void clock_sleep(const unsigned sleep_time_ms)
{
    esp_sleep_enable_timer_wakeup((uint64_t)sleep_time_ms * 1000); 
    esp_deep_sleep_start();
}

int clock_set_pin(int pin, unsigned state)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT);
    return gpio_set_level(pin, state);
}

