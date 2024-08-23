
#include <stdio.h>
#include "sound_generator.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "device_common.h"
#include "screen_handler.h"





void app_main() 
{
    device_common_init();
    start_signale_series(40, 3, 1000);
    vTaskDelay(100);
    tasks_init();
}









