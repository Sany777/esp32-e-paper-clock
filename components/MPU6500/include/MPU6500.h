#ifndef MPU6500_H
#define MPU6500_H

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MPU6500_EN_PIN 19
#define MPU6500_INT_PIN 4    

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
    int x_angle; 
    int y_angle; 
} MPU6500_data_t;

int MPU6500_init();
int MPU6500_read_data(MPU6500_data_t *mpu_data);
void MPU6500_on();
void MPU6500_off();


#ifdef __cplusplus
}
#endif





#endif