#ifndef HC905A_H_
#define HC905A_H_

#ifdef __cplusplus
extern "C" {
#endif

void buzer_start();
void start_signale(int delay, int count);
void buzer_stop();

#ifdef __cplusplus
}
#endif

#define SIG_OUT_PIN          (18) 







#endif