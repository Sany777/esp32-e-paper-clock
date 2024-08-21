
#include <stdio.h>
#include "sound_generator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "device_system.h"
#include "screen_handler.h"





void app_main() 
{
    device_system_init();
    start_signale_series(40, 3, 1000, 10);
    vTaskDelay(100);
    tasks_init();
}









