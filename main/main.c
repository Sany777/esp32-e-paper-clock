
#include <stdio.h>
#include "AHT21.h"
#include "esp_log.h"
#include "HC905A.h"
#include "adc_reader.h"
#include "wifi_service.h"
#include "setting_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "device_gpio.h"
#include "MPU6500.h"
#include "device_system.h"


#include "epaper_adapter.h"




void app_main() 
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    device_system_init();




    vTaskDelay(200);

    // start_ap();
    // start_server();

    while (1) {
        mpu_measure();
        int pos = mpu_get_rotate();
        if(pos < 4){
        epaper_set_rotate(pos);
        }
        buzer_start();
        float temperature = 0, humidity = 0;
        AHT21_read_data(&temperature, &humidity);
        epaper_printf(10,10,24, "%.2f*C", temperature);
        epaper_printf(10,50,24, "%.2f%%", humidity);

        float volt = adc_reader_get_voltage();
        epaper_printf(10,90,24, "%.2fV", volt);
        device_sleep(59000);
    }

}









