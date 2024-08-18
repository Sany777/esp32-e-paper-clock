
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
    vTaskDelay(200);
    tasks_init();
    
    // while (1) 
    // {
    //     // mpu_measure();

    //         // int cmd = device_get_joystick_btn();
    //         // if(cmd != NO_DATA){
    //         //     start_signale_series(50, 5, 15);
    //         //     device_set_state(BIT_START_UPDATE_SCR);
    //         // }
    // //     // start_signale();
    // //     // float temperature = 0, humidity = 0;
    // //     // AHT21_read_data(&temperature, &humidity);
    // //     // epaper_printf(10,40,24, COLORED, "%.2f*C", temperature);
    // //     // epaper_printf(10,70,24, UNCOLORED, "%.2f%%", humidity);
    // //     // draw_horizontal_line(10,150, 75, 5, COLORED);
    // //     // float volt = adc_reader_get_voltage();
    // //     // epaper_printf(10,100,24,COLORED, "%.2fV", volt);
    // //     // draw_circle(100, 100, 15, COLORED, false);
    // //     // draw_circle(100, 100, 10, COLORED, true);
    // //     // epaper_display();
    //     vTaskDelay(2000/portTICK_PERIOD_MS);
    //     device_sleep(20000);
    // }

}









