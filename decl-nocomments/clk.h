#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int64_t freq_hz;
    int ticks_to_run;
    int overrun_ticks;
} clk_t;

extern void clk_init(clk_t* clk, uint32_t freq_hz);
extern uint32_t clk_ticks_to_run(clk_t* clk, uint32_t micro_seconds);
extern void clk_ticks_executed(clk_t* clk, uint32_t ticks);

#ifdef __cplusplus
} /* extern "C" */
#endif
