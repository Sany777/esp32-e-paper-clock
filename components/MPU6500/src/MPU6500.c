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

static const char *TAG = "MPU6500";

#define MPU6500_ADDR             0x68
#define MPU6500_PWR_MGMT_1       0x6B
#define MPU6500_SMPLRT_DIV       0x19
#define MPU6500_CONFIG           0x1A
#define MPU6500_GYRO_CONFIG      0x1B
#define MPU6500_ACCEL_CONFIG     0x1C
#define MPU6500_ACCEL_XOUT_H     0x3B
#define MPU6500_TEMP_OUT_H       0x41
#define MPU6500_GYRO_XOUT_H      0x43
#define MPU6500_INT_ENABLE       0x38
#define MPU6500_PWR_MGMT_1_SLEEP 0x40
#define MPU6500_PWR_MGMT_1_WAKEUP 0x00


#define ACCEL_SCALE 16384.0  // Значення масштабу для акселерометра (±2g / 16-bit)


int MPU6500_init() 
{
    ESP_LOGI(TAG, "Initializing MPU6500");
    MPU6500_off();
    vTaskDelay(pdMS_TO_TICKS(500));
    MPU6500_on();
    // Wake up the MPU6500 since it starts in sleep mode
    if (I2C_write_reg(MPU6500_ADDR, MPU6500_PWR_MGMT_1, MPU6500_PWR_MGMT_1_WAKEUP) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wake up MPU6500");
        return ESP_FAIL;
    }

    // Set sample rate to 1kHz
    if (I2C_write_reg(MPU6500_ADDR, MPU6500_SMPLRT_DIV, 0x07) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set sample rate");
        return ESP_FAIL;
    }

    // Set gyro configuration (±250 degrees per second)
    if (I2C_write_reg(MPU6500_ADDR, MPU6500_GYRO_CONFIG, 0x00) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set gyro configuration");
        return ESP_FAIL;
    }

    // Set accelerometer configuration (±2g)
    if (I2C_write_reg(MPU6500_ADDR, MPU6500_ACCEL_CONFIG, 0x00) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set accelerometer configuration");
        return ESP_FAIL;
    }

    // Enable interrupt
    if (I2C_write_reg(MPU6500_ADDR, MPU6500_INT_ENABLE, 0x01) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable interrupt");
        return ESP_FAIL;
    }

    return ESP_OK;
}

int MPU6500_read_data(MPU6500_data_t *mpu_data) 
{
    uint8_t data[14];
    
    // Read 14 bytes from MPU6500 starting from the accelerometer XOUT high register
    if (I2C_read_reg(MPU6500_ADDR, MPU6500_ACCEL_XOUT_H, data, sizeof(data)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data from MPU6500");
        return ESP_FAIL;
    }

    // Process the received data
    mpu_data->accel_x = (int16_t)((data[0] << 8) | data[1]);
    mpu_data->accel_y = (int16_t)((data[2] << 8) | data[3]);
    mpu_data->accel_z = (int16_t)((data[4] << 8) | data[5]);
    mpu_data->temp = (int16_t)((data[6] << 8) | data[7]);
    mpu_data->gyro_x = (int16_t)((data[8] << 8) | data[9]);
    mpu_data->gyro_y = (int16_t)((data[10] << 8) | data[11]);
    mpu_data->gyro_z = (int16_t)((data[12] << 8) | data[13]);
   // Перерахунок даних в градації
    float accel_x = mpu_data->accel_x / ACCEL_SCALE;
    float accel_y = mpu_data->accel_y / ACCEL_SCALE;
    float accel_z = mpu_data->accel_z / ACCEL_SCALE;

    // Розрахунок нахилу в градусах
    mpu_data->y_angle = atan2f(accel_y, sqrtf(accel_x * accel_x + accel_z * accel_z)) * (180.0 / M_PI);
    mpu_data->x_angle = atan2f(-accel_x, accel_z) * (180.0 / M_PI);

    return ESP_OK;
}

void MPU6500_on()
{
    set_pin(MPU6500_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
}

void MPU6500_off()
{
    set_pin(MPU6500_EN_PIN, 0);
}