#ifndef AHT21_H
#define AHT21_H


#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif

int AHT21_init();
int AHT21_read_data(float *temperature, float *humidity);
void AHT21_off();
int AHT21_on();


#ifdef __cplusplus
}
#endif








#endif