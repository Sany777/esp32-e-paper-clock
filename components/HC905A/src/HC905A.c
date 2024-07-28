#include "HC905A.h"


#define SOC_LEDC_SUPPORT_HS_MODE 1 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <math.h>

#include "additional_functions.h"
#include "periodic_taks.h"


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT 
#define LEDC_DUTY               (1000) //  50%
#define LEDC_FREQUENCY          (2000) // 2.6 кГц


static bool buzzer_init;
static int _delay;

static ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .timer_num        = LEDC_TIMER,
    .duty_resolution  = LEDC_DUTY_RES,
    .freq_hz          = LEDC_FREQUENCY,
    .clk_cfg          = LEDC_AUTO_CLK
};
static ledc_channel_config_t ledc_channel = {
    .speed_mode     = LEDC_MODE,
    .channel        = LEDC_CHANNEL,
    .timer_sel      = LEDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = SIG_OUT_PIN,
    .duty           = 0, 
    .hpoint         = 0
};

void buzer_stop()
{
    ledc_timer_pause(ledc_timer.speed_mode, ledc_timer.timer_num);
}

void init_pwm(void)
{
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ESP_ERROR_CHECK(ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LEDC_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel));
    buzzer_init = true;
}

void buzer_start()
{
    if(!buzzer_init){
        init_pwm();
    } else { 
        ledc_timer_resume(ledc_timer.speed_mode, ledc_timer.timer_num);
    }
    if(_delay == 0){
        _delay = 20;
    }
    periodic_task_remove(buzer_stop);
    periodic_task_in_isr_create(buzer_stop, _delay/2, 1);
}

void start_signale(unsigned  delay, unsigned  count)
{
    _delay = _delay;
    --count;
    if(count){
        periodic_task_in_isr_create(buzer_start, _delay, count);
    }
    buzer_start();
}




typedef enum {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE
} waveform_t;

void generate_waveform(waveform_t waveform, int sample_rate) {
    for (int i = 0; i < sample_rate; i++) {
        uint8_t dac_value = 0;

        switch (waveform) {
            case WAVEFORM_SINE:
                {
                    // float sin_val = sinf(2 * PI * i / sample_rate);
                    // dac_value = (uint8_t)((sin_val + 1) * 127.5); // Перетворення у діапазон [0, 255]
                }
                break;

            case WAVEFORM_SQUARE:
                if (i < sample_rate / 2) {
                    dac_value = 255; // Верхній рівень
                } else {
                    dac_value = 0; // Нижній рівень
                }
                break;

            case WAVEFORM_TRIANGLE:
                if (i < sample_rate / 2) {
                    dac_value = (uint8_t)(i * 255 / (sample_rate / 2)); // Лінійне збільшення
                } else {
                    dac_value = (uint8_t)((sample_rate - i) * 255 / (sample_rate / 2)); // Лінійне зменшення
                }
                break;
        }


        // vTaskDelay(portTICK_PERIOD_MS(sample_rate));
    }
}
