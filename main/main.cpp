#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"
#include "additional_functions.h"

#include <stdio.h>
#include "AHT21.h"
#include "MPU6500.h"
#include "additional_functions.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "HC905A.h"
#include "joystick.h"
#include "adc_reader.h"
#include "epaper_adaper.h"


static const char *TAG = "main";

EPaperAdapter epaper;

extern "C" void app_main() 
{

    // epaper.init();
  joystick_init();
  adc_reader_init();


  if (AHT21_init() != ESP_OK) {
  ESP_LOGE(TAG, "Failed to initialize AHT21");
  // return;
  }
  

  buzer_start();
  vTaskDelay(200);

    while (1) {
        // MPU6500_read_data(&mpu_data);



        // float temperature = 0, humidity = 0;
        // if (AHT21_read_data(&temperature, &humidity) == ESP_OK) {
        //     ESP_LOGI(TAG, "Temperature: %.2f°C, Humidity: %.2f%%", temperature, humidity);


        // } else {
        //     ESP_LOGE(TAG, "Failed to read data from AHT21");
        // }
        // adc_reader_get_voltage();
        if(get_joystick_btn() != -1){
            buzer_start();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}









