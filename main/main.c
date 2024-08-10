
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


void set_screen()
{

}

void app_main() 
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    
    device_system_init();
    epaper_clear(UNCOLORED);
    vTaskDelay(200);
    

    while (1) 
    {

        int pos = mpu_get_rotate();
        if(pos <4){
            epaper_set_rotate(pos);
        }
        start_signale();
        float temperature = 0, humidity = 0;
        AHT21_read_data(&temperature, &humidity);
        epaper_printf(10,10,24, COLORED, "%.2f*C", temperature);
        epaper_printf(10,50,24, UNCOLORED, "%.2f%%", humidity);
        draw_horizontal_line(10,150, 75, 5, COLORED);
        float volt = adc_reader_get_voltage();
        epaper_printf(10,90,24,COLORED, "%.2fV", volt);
        draw_circle(100, 100, 15, COLORED, false);
        draw_circle(100, 100, 10, COLORED, true);
        epaper_display();
        device_sleep(10000);
    }

}









