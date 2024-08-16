#ifndef MPU6500_H
#define MPU6500_H



#ifdef __cplusplus
extern "C" {
#endif
  


enum Position{
    TURN_NORMAL,
    TURN_RIGHT,
    TURN_UPSIDE_DOWN,
    TURN_LEFT,
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
int get_rotate_num();
void mpu_init_pos();



#ifdef __cplusplus
}
#endif




#endif