
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




    buzer_start();
    vTaskDelay(200);

    // start_ap();
    // start_server();

    while (1) {
        float temperature = 0, humidity = 0;
        AHT21_read_data(&temperature, &humidity);
        // epaper.printf(50,50,24, "%.2f°C", temperature);
        // epaper.printf(10,80,24, "%.2f%%", humidity);

        // mpu.read_data();
        // int pos = mpu.get_rotate();
        // if(pos < 4){
        //     epaper.set_rotate(pos);
        // }
        // epaper.refresh();
        adc_reader_get_voltage();
        if(device_get_joystick_btn() != -1){
            buzer_start();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}









