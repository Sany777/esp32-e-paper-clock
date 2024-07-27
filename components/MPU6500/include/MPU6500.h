#ifndef MPU6500_H
#define MPU6500_H

#include "stdint.h"

// #ifdef __cplusplus
// extern "C" {
// #endif


#define MPU6500_EN_PIN 19
#define MPU6500_INT_PIN 4   

enum Position{
    TURN_NORMAL,
    TURN_RIGHT,
    TURN_LEFT,
    TURN_UPSIDE_DOWN,
    TURN_UP,
    TURN_DOWN,
};

class MPU6500 {
public:
    void on();
    void off();
    int init();
    int read_data();
    int get_x(){return x_angle;}
    int get_y(){return y_angle;}
    int get_temp(){return temperature;}
    int get_pos(int x, int y);
    int get_pos(){return pos;}
    ~MPU6500(){off();}
    const char* pos_to_str(int p);
    
private:
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
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

};





// #ifdef __cplusplus
// }
// #endif





#endif