#ifndef HC905A_H_
#define HC905A_H_

#ifdef __cplusplus
extern "C" {
#endif

void buzer_start();
void start_signale(unsigned  delay, unsigned  count, int loud);
void buzer_stop();

void buz_test(int freq_hz);


#ifdef __cplusplus
}
#endif









#endif