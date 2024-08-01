extern "C"{
    #include <stdio.h>
    #include "AHT21.h"
    #include "MPU6500.h"
    #include "additional_functions.h"
    #include "esp_log.h"
    #include "HC905A.h"
    #include "joystick.h"
    #include "adc_reader.h"
    #include "wifi_service.h"
    #include "setting_server.h"
    #include "clock_system.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "i2c_module.h"
}

#include "epaper_adaper.h"

static const char *TAG = "main";

EPaperAdapter epaper;

extern "C" void app_main() 
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    I2C_init();
    device_system_init();
    epaper.init();
    epaper.printf(10,80,24, "Hello");
    joystick_init();
    adc_reader_init();
    MPU6500 mpu;
    mpu.init();
    if (AHT21_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize");
    // return;
    }


    buzer_start();
    vTaskDelay(200);

    // start_ap();
    // start_server();

    while (1) {
        float temperature = 0, humidity = 0;
        AHT21_read_data(&temperature, &humidity);
        epaper.printf(50,50,24, "%.2f°C", temperature);
        epaper.printf(10,80,24, "%.2f%%", humidity);

        mpu.read_data();
        int pos = mpu.get_rotate();
        if(pos < 4){
            epaper.set_rotate(pos);
        }
        epaper.refresh();
        adc_reader_get_voltage();
        if(get_joystick_btn() != -1){
            buzer_start();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}









