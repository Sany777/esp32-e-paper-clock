#include "sound_generator.h"


#define SOC_LEDC_SUPPORT_HS_MODE 1 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include <math.h>

#include "device_gpio.h"
#include "periodic_taks.h"


#define LEDC_TIMER              LEDC_TIMER_2
#define LEDC_MODE               LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUT_RES            LEDC_TIMER_13_BIT 
#define DEFAULT_DUTY            (50) //  50%
#define DEFAULT_FREQUENCY       (1200) // 2.6 кГц


static bool buzzer_init, buzzer_work;
static unsigned _delay, _freq_hz, _duty;

static ledc_timer_config_t ledc_timer = {
    .speed_mode       = LEDC_MODE,
    .timer_num        = LEDC_TIMER,
    .duty_resolution  = LEDC_DUT_RES,
    .freq_hz          = 0,
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

static void signale_stop();
static void start_pwm();
static void init_pwm();



void set_sound_freq(unsigned freq_hz)
{
    _freq_hz = freq_hz;
    buzzer_init = false;
}

void set_sound_loud(unsigned duty)
{
    _duty = duty;
    buzzer_init = false;
}

void set_sound_delay(unsigned delay)
{
    _delay = delay;
}

void start_signale()
{
    if(!buzzer_init){
        init_pwm();
        start_pwm();
    } else {
        ledc_timer_resume(ledc_timer.speed_mode, ledc_timer.timer_num);
    }
    if(_delay == 0){
        _delay = 100;
    }
    periodic_task_isr_create(signale_stop, _delay/2, 1);
}

void start_signale_series(unsigned delay, unsigned number, unsigned loud)
{
    _duty = loud;
    _delay = delay;
    start_signale();
    if(number>1){
        periodic_task_isr_create(start_signale, _delay, number-1);
    }
}

static void signale_stop()
{
    ledc_timer_pause(ledc_timer.speed_mode, ledc_timer.timer_num);
    buzzer_work = false;
}

static void init_pwm()
{
    bool restart = false;
    if(buzzer_work){
        restart = true;
        signale_stop();
    }
    if(_freq_hz == 0)_freq_hz = DEFAULT_FREQUENCY;
    ledc_timer.freq_hz = _freq_hz;
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    buzzer_init = true;
    if(restart){
        start_pwm();
    }
}

static void start_pwm()
{
    if(buzzer_work){
        signale_stop();
    }
    if(_duty>= 100 || _duty == 0) _duty = DEFAULT_DUTY;
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, _duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
    buzzer_work = true;
}
