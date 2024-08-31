#include "MPU6500.h"


#include "i2c_adapter.h"
#include "string.h"
#include "stdint.h"
#include <stdio.h>
#include "esp_log.h"
#include "soc/gpio_struct.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "math.h"
#include "device_common.h"
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
#define MPU6500_PWR_MGMT_2       0x6C
#define CYCLE_BIT               (1<<5)
#define DIS_XG                  (1<<2)
#define DIS_YG                  (1<<1)
#define DIS_ZG                  (1<<0)
#define MPU6500_CONFIG          0x1A
#define MPU6500_GYRO_CONFIG     0x1B
#define MPU6500_ACCEL_CONFIG    0x1C
#define MPU6500_ACCEL_CONFIG_2  0xD
#define A_DLPFCFG_BIT           (1<<0)
#define ACCEL_FCHOICE_B         (1<<1)
#define MPU6500_LP_ACCEL_ODR    0x1E    // data 0 = 0.24Hz, 6 = 7.81Hz, 11 = 500 Hz
#define MPU6500_WOM_THR         0x1F    // data 0bxxxxxxx
#define INT_PIN_CFG             0x37
// ACTL_BIT = 1 : intr = high
#define ACTL_BIT                (1<<7)
//OPEN_BIT = 1 : INT pin is configured as open drain. 0 – INT pin is configured as push-pull.
#define OPEN_BIT                (1<<6)
//0 – INT pin indicates interrupt pulse’s is width 50µs.
#define INT_ENABLE              0x38
#define LATCH_INT_EN_BIT        (1<<5)
//1 – Wake on motion interrupt occurred
#define WOM_EN_BIT              (1<<6)
#define RAW_RDY_EN_BIT          (1<<0)
#define ACCEL_INTEL_CTRL        0x69
#define ACCEL_INTEL_EN_BIT      (1<<7)
#define ACCEL_INTEL_MODE_BIT    (1<<6)

#define MPU6500_ACCEL_XOUT_H     0x3B

#define ACCEL_SCALE 16384.0 

int IRAM_ATTR mpu_get_x(){return x_angle;}
int IRAM_ATTR mpu_get_y(){return y_angle;}
int IRAM_ATTR mpu_get_temp(){return temperature;}
int IRAM_ATTR mpu_get_rotate(){ mpu_measure(); return pos;}


static int IRAM_ATTR mpu_get_rotate_pos(int x, int y);

int mpu_init() 
{
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_PWR_MGMT_1, 0));
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_PWR_MGMT_2, A_DLPFCFG_BIT);
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, INT_ENABLE, 0x40)));
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, ACCEL_INTEL_CTRL, ACCEL_INTEL_EN_BIT|ACCEL_INTEL_MODE_BIT));
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_WOM_THR, 0x10));
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_LP_ACCEL_ODR, 2));  // 2 = 0.98 Hz
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_PWR_MGMT_1, CYCLE_BIT));
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_CONFIG, 0x03));  // Пропускний фільтр 44 гц
    CHECK_AND_RET_ERR(I2C_write_reg(MPU6500_ADDR, MPU6500_GYRO_CONFIG, 0x00)); 
    mpu_measure();
    return ESP_OK;
}

const char* mpu_pos_to_str(int p)
{
    if(p>= 6 || p <0)return "NO DATA";
    static const char* pos_str[6] = {
        "NORMAL",
        "RIGHT",
        "LEFT",
        "UPSIDE DOWN",
        "UP",
        "DOWN", 
    };
    return pos_str[p];
}

int IRAM_ATTR mpu_get_rotate_pos(int x, int y)
{
    if(y<38 && y>=23 && x>-10 && x<=-1)return TURN_UP;
    if(y>=18 && y<= 30 && x<=-37 && x>=-43)return TURN_DOWN;
    if(y>=18 && y<= 26 && x<=-13 && x>= -24)return  TURN_UPSIDE_DOWN;
    if(y>=30 && y<=42 && x<=-29 && x>=-43)return TURN_RIGHT;
    if(y>=8 && y<=12 && x>=-31 && x<=-22)return TURN_LEFT;
    return TURN_NORMAL;
}

int mpu_measure() 
{
    uint8_t data[14];
    CHECK_AND_RET_ERR(I2C_read_reg(MPU6500_ADDR, MPU6500_ACCEL_XOUT_H, data, sizeof(data)));
    accel_x = (uint16_t)((data[0] << 8) | data[1]) ;
    accel_y = (uint16_t)((data[2] << 8) | data[3]);
    accel_z = (uint16_t)((data[4] << 8) | data[5]);
    temperature = (uint16_t)((data[6] << 8) | data[7]);
    gyro_x = (uint16_t)((data[8] << 8) | data[9]);
    gyro_y = (uint16_t)((data[10] << 8) | data[11]);
    gyro_z = (uint16_t)((data[12] << 8) | data[13]);

    float x =   accel_x / ACCEL_SCALE;
    float y =   accel_y / ACCEL_SCALE;
    float z =   accel_z / ACCEL_SCALE;

    y_angle = atan2f(y, sqrtf(x * x + z * z)) * (180.0 / M_PI);
    x_angle = atan2f(-x, z) * (180.0 / M_PI);
    pos = mpu_get_rotate_pos(x_angle, y_angle);
    // ESP_LOGI("", "y_angle: %d, x_angle:%d - %s\n", y_angle, x_angle, mpu_pos_to_str(pos));
    return ESP_OK;
}



int mpu_on()
{
    device_set_pin(PIN_MPU6500_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(300));
    return mpu_init();
}

void mpu_off()
{
    device_set_pin(PIN_MPU6500_EN, 0);
    is_init = false;
}