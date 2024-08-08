#ifndef SOUND_GENERATOR_H_
#define SOUND_GENERATOR_H_

#ifdef __cplusplus
extern "C" {
#endif

void start_signale();
void start_signale_series(unsigned  delay, unsigned  count, unsigned loud);
void set_sound_freq(unsigned freq_hz);
void set_sound_loud(unsigned duty);
void set_sound_delay(unsigned delay);

#ifdef __cplusplus
}
#endif









#endif