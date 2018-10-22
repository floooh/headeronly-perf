#pragma once
/*#
    # m6581.h

    MOS Technology 6581 emulator (aka SID)

    based on tedplay (c) 2012 Attila Grosz, used under Unlicense http://unlicense.org/

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    ~~~C    
    CHIPS_ASSERT(c)
    ~~~

    ## Emulated Pins

    ***********************************
    *           +-----------+         *
    *    CS --->|           |<--- A0  *
    *    RW --->|           |...      *
    *           |           |<--- A4  *
    *           |   m6581   |         *
    *           |           |<--> D0  *
    *           |           |...      *
    *           |           |<--> D7  *
    *           |           |         *
    *           +-----------+         *§
    ***********************************

    TODO: Documentation

    ## Links

    - http://blog.kevtris.org/?p=13

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* address bus pins A0..A4 */
#define M6581_A0    (1ULL<<0)
#define M6581_A1    (1ULL<<1)
#define M6581_A2    (1ULL<<2)
#define M6581_A3    (1ULL<<3)
#define M6581_A4    (1ULL<<4)
#define M6581_ADDR_MASK (0x1F)

/* data bus pins D0..D7 */
#define M6581_D0    (1ULL<<16)
#define M6581_D1    (1ULL<<17)
#define M6581_D2    (1ULL<<18)
#define M6581_D3    (1ULL<<19)
#define M6581_D4    (1ULL<<20)
#define M6581_D5    (1ULL<<21)
#define M6581_D6    (1ULL<<22)
#define M6581_D7    (1ULL<<23)

/* shared control pins */
#define M6581_RW    (1ULL<<24)      /* same as M6502_RW */

/* chip-specific pins */
#define M6581_CS    (1ULL<<40)      /* chip-select */

/* registers */
#define M6581_V1_FREQ_LO    (0)
#define M6581_V1_FREQ_HI    (1)
#define M6581_V1_PW_LO      (2)
#define M6581_V1_PW_HI      (3)
#define M6581_V1_CTRL       (4)
#define M6581_V1_ATKDEC     (5)
#define M6581_V1_SUSREL     (6)
#define M6581_V2_FREQ_LO    (7)
#define M6581_V2_FREQ_HI    (8)
#define M6581_V2_PW_LO      (9)
#define M6581_V2_PW_HI      (10)
#define M6581_V2_CTRL       (11)
#define M6581_V2_ATKDEC     (12)
#define M6581_V2_SUSREL     (13)
#define M6581_V3_FREQ_LO    (14)
#define M6581_V3_FREQ_HI    (15)
#define M6581_V3_PW_LO      (16)
#define M6581_V3_PW_HI      (17)
#define M6581_V3_CTRL       (18)
#define M6581_V3_ATKDEC     (19)
#define M6581_V3_SUSREL     (20)
#define M6581_FC_LO         (21)
#define M6581_FC_HI         (22)
#define M6581_RES_FILT      (23)
#define M6581_MODE_VOL      (24)
#define M6581_POT_X         (25)
#define M6581_POT_Y         (26)
#define M6581_OSC3RAND      (27)
#define M6581_ENV3          (28)
#define M6581_INV_0         (29)
#define M6581_INV_1         (30)
#define M6581_INV_2         (31)
#define M6581_NUM_REGS (32)

/* voice control bits */
#define M6581_CTRL_GATE     (1<<0)
#define M6581_CTRL_SYNC     (1<<1)
#define M6581_CTRL_RINGMOD  (1<<2)
#define M6581_CTRL_TEST     (1<<3)
#define M6581_CTRL_TRIANGLE (1<<4)
#define M6581_CTRL_SAWTOOTH (1<<5)
#define M6581_CTRL_PULSE    (1<<6)
#define M6581_CTRL_NOISE    (1<<7)

/* filter routing bits */
#define M6581_FILTER_FILT1  (1<<0)
#define M6581_FILTER_FILT2  (1<<1)
#define M6581_FILTER_FILT3  (1<<2)
#define M6581_FILTER_FILTEX (1<<3)

/* filter mode bits */
#define M6581_FILTER_LP     (1<<0)
#define M6581_FILTER_BP     (1<<1)
#define M6581_FILTER_HP     (1<<2)
#define M6581_FILTER_3OFF   (1<<3)

/* setup parameters for m6581_init() */
typedef struct {
    int tick_hz;        /* frequency at which m6581_tick() will be called in Hz */
    int sound_hz;       /* sound sample frequency */
    float magnitude;    /* output sample magnitude (0=silence to 1=max volume) */
} m6581_desc_t;

/* envelope generator state */
typedef enum {
    M6581_ENV_FROZEN,
    M6581_ENV_ATTACK,
    M6581_ENV_DECAY,
    M6581_ENV_RELEASE
} _m6581_env_state_t;

/* voice state */
typedef struct {
    bool muted;
    /* wave generator state */
    uint16_t freq;
    uint16_t pulse_width;
    uint8_t ctrl;
    bool sync;
    uint32_t noise_shift;           /* 24 bit */
    uint32_t wav_accum;             /* 8.16 fixed */
    uint32_t wav_output;            /* 12 bit */

    /* envelope generator state */
    _m6581_env_state_t env_state;
    uint32_t env_attack_add;
    uint32_t env_decay_sub;
    uint32_t env_sustain_level;
    uint32_t env_release_sub;
    uint32_t env_cur_level;
    uint32_t env_counter;
    uint32_t env_exp_counter;
    uint32_t env_counter_compare;
} m6581_voice_t;

/* filter state */
typedef struct {
    uint16_t cutoff;
    uint8_t resonance;
    uint8_t voices;
    uint8_t mode;
    uint8_t volume;
    int nyquist_freq;
    int resonance_coeff_div_1024;
    int w0;
    int v_hp;
    int v_bp;
    int v_lp;
} m6581_filter_t;

/* m6581 instance state */
typedef struct {
    int sound_hz;
    /* reading a write-only register returns the last value
       written to *any* register for about 0x2000 ticks
    */
    uint8_t bus_value;
    uint16_t bus_decay;
    /* voice state */
    m6581_voice_t voice[3];
    /* filter state */
    m6581_filter_t filter;
    /* sample generation state */
    int sample_period;
    int sample_counter;
    float sample_accum;
    float sample_accum_count;
    float sample_mag;
    float sample;
} m6581_t;

/* initialize a new m6581_t instance */
extern void m6581_init(m6581_t* sid, m6581_desc_t* desc);
/* reset a m6581_t instance */
extern void m6581_reset(m6581_t* sid);
/* read/write m6581_t registers */
extern uint64_t m6581_iorq(m6581_t* sid, uint64_t pins);
/* tick a m6581_t instance, returns true when new sample is ready */
extern bool m6581_tick(m6581_t* sid);

#ifdef __cplusplus
} /* extern "C" */
#endif
