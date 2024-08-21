
#include <stdio.h>
#include "AHT21.h"
#include "esp_log.h"
#include "sound_generator.h"
#include "adc_reader.h"
#include "wifi_service.h"
#include "setting_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "device_system.h"
#include "MPU6500.h"
#include "device_system.h"
#include "screen_handler.h"

#include "epaper_adapter.h"


void set_screen()
{

}

void app_main() 
{

    device_system_init();
    start_signale_series(40, 3, 1000, 10);
    vTaskDelay(100);
    tasks_init();
}









