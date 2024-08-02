#include "adc_reader.h"

#include <stdio.h>
#include "driver/adc.h"
#include "esp_log.h"
#include "adc_reader.h"
#include "device_gpio.h"




#define ADC_CHANNEL ADC1_CHANNEL_4  // ADC1 channel 4 is GPIO 13
#define ADC_ATTEN ADC_ATTEN_DB_6    // 11 dB attenuation (voltage range 0 - 3.9V)
#define ADC_MAX_VALUE 4095          // 12-bit ADC maximum value
#define ADC_REF_VOLTAGE 8.4         // Reference voltage for 11 dB attenuation

static const char *TAG = "ADC_READER";


void adc_reader_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);
    ESP_LOGI(TAG, "ADC Initialized");
}


float adc_reader_get_voltage(void)
{
    int num = MESUR_NUM;
    int adc_value = 0;
    while(num--){
        adc_value += adc1_get_raw(ADC_CHANNEL);
    }
    adc_value /= MESUR_NUM;
    float voltage = ((float)adc_value / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    ESP_LOGI(TAG, "ADC Value: %d, Voltage: %.2fV", adc_value, voltage);
    return voltage;
}
