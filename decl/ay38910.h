#pragma once
/*
    ay38910.h   -- AY-3-8910/2/3 sound chip emulator

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provde the following macros with your own implementation
    
    CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

             +-----------+
      BC1 -->|           |<-> DA7
     BDIR -->|           |...
             |           |<-> DA7
             |           |
             |           |<-> (IOA7)
             |           |...
             |           |<-> (IOA0)
             |           |
             |           |<-> (IOB7)
             |           |...
             |           |<-> (IOB0)
             +-----------+

    NOT EMULATED:

    - the BC2 pin is ignored since it makes only sense when connected to
      a CP1610 CPU
    - the RESET pin state is ignored, instead call ay38910_reset()

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
*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Pin definitions.

    Note that the BC2 is not emulated since it is usually always
    set to active when not connected to a CP1610 processor. The
    remaining BDIR and BC1 pins are interprested as follows:

    |BDIR|BC1|
    +----+---+
    |  0 | 0 |  INACTIVE
    |  0 | 1 |  READ FROM PSG
    |  1 | 0 |  WRITE TO PSG
    |  1 | 1 |  LATCH ADDRESS
*/

/* 8 bits data/address bus shared with CPU data bus */
#define AY38910_DA0 (1ULL<<16)
#define AY38910_DA1 (1ULL<<17)
#define AY38910_DA2 (1ULL<<18)
#define AY38910_DA3 (1ULL<<19)
#define AY38910_DA4 (1ULL<<20)
#define AY38910_DA5 (1ULL<<21)
#define AY38910_DA6 (1ULL<<22)
#define AY38910_DA7 (1ULL<<23)

/* reset pin shared with CPU */
#define AY38910_RESET   (1ULL<<34)

/* chip-specific pins start at position 44 */
#define AY38910_BDIR    (1ULL<<44)
#define AY38910_BC1     (1ULL<<45)
#define AY38910_A8      (1ULL<<46)

/* AY-3-8910 registers */
#define AY38910_REG_PERIOD_A_FINE       (0)
#define AY38910_REG_PERIOD_A_COARSE     (1)
#define AY38910_REG_PERIOD_B_FINE       (2)
#define AY38910_REG_PERIOD_B_COARSE     (3)
#define AY38910_REG_PERIOD_C_FINE       (4)
#define AY38910_REG_PERIOD_C_COARSE     (5)
#define AY38910_REG_PERIOD_NOISE        (6)
#define AY38910_REG_ENABLE              (7)
#define AY38910_REG_AMP_A               (8)
#define AY38910_REG_AMP_B               (9)
#define AY38910_REG_AMP_C               (10)
#define AY38910_REG_ENV_PERIOD_FINE     (11)
#define AY38910_REG_ENV_PERIOD_COARSE   (12)
#define AY38910_REG_ENV_SHAPE_CYCLE     (13)
#define AY38910_REG_IO_PORT_A           (14)    /* not on AY-3-8913 */
#define AY38910_REG_IO_PORT_B           (15)    /* not on AY-3-8912/3 */
/* number of registers */
#define AY38910_NUM_REGISTERS (16)
/* error-accumulation precision boost */
#define AY38910_FIXEDPOINT_SCALE (16)
/* number of channels */
#define AY38910_NUM_CHANNELS (3)

/* IO port names */
#define AY38910_PORT_A (0)
#define AY38910_PORT_B (1)

/* envelope shape bits */
#define AY38910_ENV_HOLD        (1<<0)
#define AY38910_ENV_ALTERNATE   (1<<1)
#define AY38910_ENV_ATTACK      (1<<2)
#define AY38910_ENV_CONTINUE    (1<<3)

/* callbacks for input/output on I/O ports */
typedef uint8_t (*ay38910_in_t)(int port_id, void* user_data);
typedef void (*ay38910_out_t)(int port_id, uint8_t data, void* user_data);

/* chip subtypes */
typedef enum {
    AY38910_TYPE_8910 = 0,
    AY38910_TYPE_8912,
    AY38910_TYPE_8913
} ay38910_type_t;

/* setup parameters for ay38910_init() call */
typedef struct {
    ay38910_type_t type;    /* the subtype (default 0 is AY-3-8910) */
    int tick_hz;            /* frequency at which ay38910_tick() will be called in Hz */
    int sound_hz;           /* number of samples that will be produced per second */
    float magnitude;        /* output sample magnitude, from 0.0 (silence) to 1.0 (max volume) */ 
    ay38910_in_t in_cb;     /* I/O port input callback */
    ay38910_out_t out_cb;   /* I/O port output callback */
    void* user_data;        /* optional user-data for callbacks */
} ay38910_desc_t;

/* a tone channel */
typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t bit;
    uint32_t tone_disable;
    uint32_t noise_disable;
} ay38910_tone_t;

/* the noise channel state */
typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t rng;
    uint32_t bit;
} ay38910_noise_t;

/* the envelope generator */
typedef struct {
    uint16_t period;
    uint16_t counter;
    bool shape_holding;
    bool shape_hold;
    uint8_t shape_counter;
    uint8_t shape_state;
} ay38910_env_t;

/* AY-3-8910 state */
typedef struct {
    ay38910_type_t type;        /* the chip flavour */
    ay38910_in_t in_cb;         /* the port-input callback */
    ay38910_out_t out_cb;       /* the port-output callback */
    void* user_data;            /* optional user-data for callbacks */
    uint32_t tick;              /* a tick counter for internal clock division */
    uint8_t addr;               /* 4-bit address latch */
    union {                     /* the register bank */
        uint8_t reg[AY38910_NUM_REGISTERS];
        struct {
            uint8_t period_a_fine;
            uint8_t period_a_coarse;
            uint8_t period_b_fine;
            uint8_t period_b_coarse;
            uint8_t period_c_fine;
            uint8_t period_c_coarse;
            uint8_t period_noise;
            uint8_t enable;
            uint8_t amp_a;
            uint8_t amp_b;
            uint8_t amp_c;
            uint8_t period_env_fine;
            uint8_t period_env_coarse;
            uint8_t env_shape_cycle;
            uint8_t port_a;
            uint8_t port_b;
        };
    };
    ay38910_tone_t tone[AY38910_NUM_CHANNELS];  /* the 3 tone channels */
    ay38910_noise_t noise;                      /* the noise generator state */
    ay38910_env_t env;                          /* the envelope generator state */

    /* sample generation state */
    int sample_period;
    int sample_counter;
    float mag;
    float sample;
} ay38910_t;

/* extract 8-bit data bus from 64-bit pins */
#define AY38910_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define AY38910_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/* initialize a AY-3-8910 instance */
extern void ay38910_init(ay38910_t* ay, ay38910_desc_t* desc);
/* reset an existing AY-3-8910 instance */
extern void ay38910_reset(ay38910_t* ay);
/* perform an IO request machine cycle */
extern uint64_t ay38910_iorq(ay38910_t* ay, uint64_t pins);
/* tick the AY-3-8910, return true if a new sample is ready */
extern bool ay38910_tick(ay38910_t* ay);

#ifdef __cplusplus
} /* extern "C" */
#endif
