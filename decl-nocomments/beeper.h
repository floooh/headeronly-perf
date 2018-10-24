#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BEEPER_FIXEDPOINT_SCALE (16)

typedef struct {
    int state;
    int period;
    int counter;
    float mag;
    float sample;
} beeper_t;

extern void beeper_init(beeper_t* beeper, int tick_hz, int sound_hz, float magnitude);
extern void beeper_reset(beeper_t* beeper);
extern void beeper_set(beeper_t* beeper, bool state);
extern void beeper_toggle(beeper_t* beeper);
extern bool beeper_tick(beeper_t* beeper);

#ifdef __cplusplus
} /* extern "C" */
#endif