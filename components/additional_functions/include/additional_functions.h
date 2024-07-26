#ifndef ADD_FUNCTIONS_H
#define ADD_FUNCTIONS_H

#include "stdint.h"
#include "esp_log.h"
#include "esp_err.h"
#include "stddef.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_EN_PIN 26

typedef void(*periodic_func_t)(void);

void stop_periodic_task(periodic_func_t func);
void new_periodic_task(periodic_func_t func, int delay_ms, int count);
int init_timer(void);
void stop_timer(void);
void tasks_run();
void esp_sleep(const uint64_t sleep_time_ms);
int set_pin(int pin, uint8_t state);


#define CHECK_AND_RET_ERR(result_)                  \
    do{                                             \
        const int e = result_;                      \
        if(e != ESP_OK){                            \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e));   \
            return e;                               \
        }                                           \
    }while(0)

#define CHECK_AND_GO(result_, label)                \
    do{                                             \
        const int e = result_;                      \
        if(e != ESP_OK){                            \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e));  \
            goto label;                             \  
        }                                           \                              
    }while(0)

#define CHECK_AND_RET(err)      \
    do{                         \
        const int e = err;      \
        if(e != ESP_OK){        \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e));  \
            return;             \
        }                       \
    }while(0)

#define delay(ms) (vTaskDelay(ms / portTICK_PERIOD_MS))

#ifdef __cplusplus
}
#endif

#endif



