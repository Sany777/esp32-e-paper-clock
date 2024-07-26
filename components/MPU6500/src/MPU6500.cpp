#include "MPU6500.h"

#include "additional_functions.h"
#include "i2c_module.h"
#include "string.h"
#include "stdint.h"
#include <stdio.h>
#include "esp_log.h"
#include "soc/gpio_struct.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "math.h"
#include "driver/gpio.h"

static const char *TAG = "MPU6500";



int MPU6500::init() 
{
    off();
    vTaskDelay(pdMS_TO_TICKS(500));
    on();
    // Wake up the MPU6500 since it starts in sleep mode
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_PWR_MGMT_1, MPU6500_PWR_MGMT_1_WAKEUP));
    // Set sample rate to 1kHz
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_SMPLRT_DIV, 0x07));
    // Set gyro configuration (±250 degrees per second)
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_GYRO_CONFIG, 0x00));
    // Set accelerometer configuration (±2g)
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_ACCEL_CONFIG, 0x00));
    // Enable interrupt
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_INT_ENABLE, 0x01));
    is_init = true;
    return ESP_OK;
}

const char* MPU6500::pos_to_str(int p)
{
    static const char* pos[] = {
        "TURN_NORMAL",
        "TURN_RIGHT",
        "TURN_LEFT",
        "TURN_UPSIDE_DOWN",
        "TURN_UP",
        "TURN_DOWN"
    };
    return pos[p];
}

int MPU6500::get_pos(int x, int y)
{
    if(y>-50 && y < 50){
        if(x>-120 && x<-60)return TURN_DOWN;
        if(x>50 && x<120)return TURN_UP;
        if(x>-25 && x<25)return TURN_LEFT;
        if(x>150 || x<-150)return TURN_RIGHT;
    }
    if(y<-55){
        return TURN_UPSIDE_DOWN;
    }
    return TURN_NORMAL;
}

int MPU6500::read_data() 
{
    if(!is_init){
        CHECK_AND_RET_ERR(init());
    }
    uint8_t data[14];
    
    // Read 14 bytes from MPU6500 starting from the accelerometer XOUT high register
    CHECK_AND_RET_ERR(I2C_read_reg(MPU6500_ADDR, MPU6500_ACCEL_XOUT_H, data, sizeof(data)));
    // Process the received data
    accel_x = (int16_t)((data[0] << 8) | data[1]) ;
    accel_y = (int16_t)((data[2] << 8) | data[3]);
    accel_z = (int16_t)((data[4] << 8) | data[5]);
    temperature = (int16_t)((data[6] << 8) | data[7]);
    gyro_x = (int16_t)((data[8] << 8) | data[9]);
    gyro_y = (int16_t)((data[10] << 8) | data[11]);
    gyro_z = (int16_t)((data[12] << 8) | data[13]);

    float x =     accel_x / ACCEL_SCALE;
    float y =     accel_y / ACCEL_SCALE;
    float z =     accel_z / ACCEL_SCALE;

    y_angle = atan2f(y, sqrtf(x * x + z * z)) * (180.0 / M_PI);
    x_angle = atan2f(-x, z) * (180.0 / M_PI);
    pos = get_pos(x_angle, y_angle);
    return ESP_OK;
}

void MPU6500::on()
{
    gpio_set_direction((gpio_num_t)MPU6500_EN_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level((gpio_num_t)MPU6500_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
}

void MPU6500::off()
{
    gpio_set_direction((gpio_num_t)MPU6500_EN_PIN, GPIO_MODE_INPUT_OUTPUT);
    gpio_set_level((gpio_num_t)MPU6500_EN_PIN, 0);
    is_init = false;
}