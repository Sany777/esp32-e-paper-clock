
#include <stdio.h>
#include "AHT21.h"
#include "esp_log.h"
#include "sound_generator.h"
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
    set_sound_loud(80);
    set_sound_delay(50);
    for(int i=100; i<6000; i+=1000){
        set_sound_freq(i);
        start_signale_series(50, 100, 40);
        ESP_LOGI("", "%d\n", i);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    while (1) {
        
        float temperature = 0, humidity = 0;
        AHT21_read_data(&temperature, &humidity);
        epaper_printf(10,10,24, "%.2f*C", temperature);
        epaper_printf(10,50,24, "%.2f%%", humidity);

        int pos = mpu_get_rotate();
        if(pos < 4){
            epaper_set_rotate(pos);
        }
        float volt = adc_reader_get_voltage();
        epaper_printf(10,90,24, "%.2fV", volt);
        device_sleep(59000);
    }

}









