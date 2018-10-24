#pragma once
#include <stdint.h>

#ifndef SOKOL_API_DECL
    #define SOKOL_API_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

SOKOL_API_DECL void stm_setup(void);
SOKOL_API_DECL uint64_t stm_now(void);
SOKOL_API_DECL uint64_t stm_diff(uint64_t new_ticks, uint64_t old_ticks);
SOKOL_API_DECL uint64_t stm_since(uint64_t start_ticks);
SOKOL_API_DECL uint64_t stm_laptime(uint64_t* last_time);
SOKOL_API_DECL double stm_sec(uint64_t ticks);
SOKOL_API_DECL double stm_ms(uint64_t ticks);
SOKOL_API_DECL double stm_us(uint64_t ticks);
SOKOL_API_DECL double stm_ns(uint64_t ticks);

#ifdef __cplusplus
} /* extern "C" */
#endif
