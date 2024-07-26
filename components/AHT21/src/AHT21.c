#include "AHT21.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_module.h"
#include "additional_functions.h"

#define AHT21_ADDR 0x38
#define AHT21_CMD_TRIGGER 0xAC
#define AHT21_CMD_SOFT_RESET 0xBA
#define AHT21_CMD_CALIBRATE 0xE1

#define AHT21_STATUS_BUSY 0x80
#define AHT21_STATUS_CALIBRATED 0x08

#define AHT21_CALIBRATE_DELAY_MS 50
#define AHT21_CHECK_BUSY_DELAY_MS 10
#define AHT21_MAX_BUSY_WAIT_MS 1000 

static const char *TAG = "AHT21";

int AHT21_init() 
{
    ESP_LOGI(TAG, "Initializing AHT21");
    AHT21_off();
    vTaskDelay(pdMS_TO_TICKS(500));
    AHT21_on();
    // Calibrate sensor
    uint8_t calib_data[] = {AHT21_CMD_CALIBRATE, 0x08, 0x00};
    CHECK_AND_RET_ERR(I2C_write_bytes(AHT21_ADDR, calib_data, sizeof(calib_data)));
    vTaskDelay(pdMS_TO_TICKS(AHT21_CALIBRATE_DELAY_MS));
    uint8_t status;
    CHECK_AND_RET_ERR(I2C_read_bytes(AHT21_ADDR,  &status, sizeof(status)));
    if (!(status & AHT21_STATUS_CALIBRATED)) {
        ESP_LOGE(TAG, "AHT21 calibration failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "AHT21 initialized successfully");
    return ESP_OK;
}

int AHT21_read_data(float *temperature, float *humidity) 
{
    uint8_t measure_data[] = {AHT21_CMD_TRIGGER, 0x33, 0x00};
    uint8_t data[6] = {0};
    uint8_t status;
    int wait_time = 0;

    CHECK_AND_RET_ERR(I2C_write_bytes(AHT21_ADDR, measure_data, sizeof(measure_data)));
    do {
        vTaskDelay(pdMS_TO_TICKS(AHT21_CHECK_BUSY_DELAY_MS));
        CHECK_AND_RET_ERR(I2C_read_response(AHT21_ADDR, &status, sizeof(status)));
        wait_time += AHT21_CHECK_BUSY_DELAY_MS;
        if (wait_time >= AHT21_MAX_BUSY_WAIT_MS) {
            ESP_LOGE(TAG, "Sensor is busy for too long");
            return ESP_FAIL;
        }
    } while (status & AHT21_STATUS_BUSY);

    CHECK_AND_RET_ERR(I2C_read_bytes(AHT21_ADDR, data, sizeof(data)));
    uint32_t raw_humidity = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);
    uint32_t raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
    *humidity = ((float)raw_humidity / 1048576.0) * 100.0;
    *temperature = ((float)raw_temperature / 1048576.0) * 200.0 - 50.0;
    return ESP_OK;
}

void AHT21_off()
{
    set_pin(AHT21_EN_PIN, 0);
}
void AHT21_on()
{
    set_pin(AHT21_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
}