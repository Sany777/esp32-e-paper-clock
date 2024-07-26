#ifndef AHT21_H
#define AHT21_H


#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif

int AHT21_init();
int AHT21_read_data(float *temperature, float *humidity);
void AHT21_off();
void AHT21_on();


#ifdef __cplusplus
}
#endif

#define AHT21_EN_PIN 23






#endif