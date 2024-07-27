#ifndef ADD_FUNCTIONS_H
#define ADD_FUNCTIONS_H

#include "stdint.h"
#include "esp_log.h"
#include "esp_err.h"
#include "stddef.h"


#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_EN_PIN 26



#define CHECK_AND_RET_ERR(result_) \
    do{ \
        const int e = result_; \
        if(e != ESP_OK){ \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e)); \
            return e; \
        } \
    }while(0)

#define CHECK_AND_GO(result_, label_) \
    do{ \
        const int e = result_; \
        if(e != ESP_OK){ \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e)); \
            goto label_; } \       
    }while(0)

#define CHECK_AND_RET(err_) \
    do{ \
        const int e = err_; \
        if(e != ESP_OK){ \
            ESP_LOGE("", "Operation failed: %s", esp_err_to_name(e)); \
            return; \
        } \
    }while(0)




void esp_sleep(const unsigned sleep_time_ms);
int set_pin(int pin, unsigned state);











#ifdef __cplusplus
}
#endif

#endif



