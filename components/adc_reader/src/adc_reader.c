#include "adc_reader.h"

#include <stdio.h>
#include "driver/adc.h"
#include "esp_log.h"
#include "adc_reader.h"
#include "device_gpio.h"

#include "freertos/FreeRTOS.h"



#define ADC_CHANNEL ADC2_CHANNEL_4  // ADC1 channel 4 is GPIO 13
#define ADC_ATTEN ADC_ATTEN_DB_12        // 11 dB attenuation (voltage range 0 - 3.9V)
#define ADC_MAX_VALUE 4095          // 12-bit ADC maximum value
#define VREF 1100 // Вольтаж референсного джерела у мілівольтах



static const char *TAG = "ADC_READER";


void adc_reader_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    ESP_LOGI(TAG, "ADC Initialized");
}


float adc_reader_get_voltage(void)
{
    // adc_reader_init();
    uint32_t res = 0;
    for(int i=0; i<MESUR_NUM; ++i){
        res += adc1_get_raw(ADC_CHANNEL);
    }
    uint32_t adc_value = res/MESUR_NUM;
    float voltage = (float)adc_value / ADC_MAX_VALUE * (VREF / 1000.0);

    ESP_LOGI(TAG, "Voltage: %.2fV", voltage);
    return voltage;
}
