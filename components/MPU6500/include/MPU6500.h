#ifndef MPU6500_H
#define MPU6500_H



#ifdef __cplusplus
extern "C" {
#endif
  

enum Position{
    TURN_NORMAL,
    TURN_RIGHT,
    TURN_LEFT,
    TURN_UPSIDE_DOWN,
    TURN_UP,
    TURN_DOWN,
};

int mpu_init();
int mpu_on();
void mpu_off();
int mpu_measure();
int mpu_get_x();
int mpu_get_y();
int mpu_get_temp();
int mpu_get_rotate();
const char* mpu_pos_to_str(int p);


#ifdef __cplusplus
}
#endif




#endif