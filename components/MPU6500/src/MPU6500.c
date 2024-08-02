#include "MPU6500.h"


#include "i2c_module.h"
#include "string.h"
#include "stdint.h"
#include <stdio.h>
#include "esp_log.h"
#include "soc/gpio_struct.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "math.h"
#include "device_gpio.h"
#include "device_macro.h"

unsigned accel_x;
unsigned accel_y;
unsigned accel_z;
unsigned gyro_x;
unsigned gyro_y;
unsigned gyro_z;
int temperature;
int x_angle; 
int y_angle; 
int pos;
bool is_init;

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

#define ACCEL_SCALE 16384.0 

int mpu_get_x(){return x_angle;}
int mpu_get_y(){return y_angle;}
int mpu_get_temp(){return temperature;}
int mpu_get_rotate(){return pos;}

static int mpu_get_rotate_pos(int x, int y);

int mpu_init() 
{
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

const char* mpu_pos_to_str(int p)
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

int mpu_get_rotate_pos(int x, int y)
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

int mpu_read_data() 
{
    if(!is_init){
        CHECK_AND_RET_ERR(mpu_on());
    }
    uint8_t data[14];
    
    // Read 14 bytes from MPU6500 starting from the accelerometer XOUT high register
    CHECK_AND_RET_ERR(I2C_read_reg(MPU6500_ADDR, MPU6500_ACCEL_XOUT_H, data, sizeof(data)));
    // Process the received data
    accel_x = ((data[0] << 8) | data[1]) ;
    accel_y = ((data[2] << 8) | data[3]);
    accel_z = ((data[4] << 8) | data[5]);
    temperature = ((data[6] << 8) | data[7]);
    gyro_x = ((data[8] << 8) | data[9]);
    gyro_y = ((data[10] << 8) | data[11]);
    gyro_z = ((data[12] << 8) | data[13]);

    float x =     accel_x / ACCEL_SCALE;
    float y =     accel_y / ACCEL_SCALE;
    float z =     accel_z / ACCEL_SCALE;

    y_angle = atan2f(y, sqrtf(x * x + z * z)) * (180.0 / M_PI);
    x_angle = atan2f(-x, z) * (180.0 / M_PI);
    pos = mpu_get_rotate_pos(x_angle, y_angle);
    return ESP_OK;
}

int mpu_on()
{
    device_set_pin(MPU6500_EN_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
    return mpu_init();
}

void mpu_off()
{
    device_set_pin(MPU6500_EN_PIN, 0);
    is_init = false;
}