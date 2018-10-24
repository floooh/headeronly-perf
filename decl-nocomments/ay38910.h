#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AY38910_DA0 (1ULL<<16)
#define AY38910_DA1 (1ULL<<17)
#define AY38910_DA2 (1ULL<<18)
#define AY38910_DA3 (1ULL<<19)
#define AY38910_DA4 (1ULL<<20)
#define AY38910_DA5 (1ULL<<21)
#define AY38910_DA6 (1ULL<<22)
#define AY38910_DA7 (1ULL<<23)

#define AY38910_RESET   (1ULL<<34)

#define AY38910_BDIR    (1ULL<<44)
#define AY38910_BC1     (1ULL<<45)
#define AY38910_A8      (1ULL<<46)

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
#define AY38910_NUM_REGISTERS (16)
#define AY38910_FIXEDPOINT_SCALE (16)
#define AY38910_NUM_CHANNELS (3)

#define AY38910_PORT_A (0)
#define AY38910_PORT_B (1)

#define AY38910_ENV_HOLD        (1<<0)
#define AY38910_ENV_ALTERNATE   (1<<1)
#define AY38910_ENV_ATTACK      (1<<2)
#define AY38910_ENV_CONTINUE    (1<<3)

typedef uint8_t (*ay38910_in_t)(int port_id, void* user_data);
typedef void (*ay38910_out_t)(int port_id, uint8_t data, void* user_data);

typedef enum {
    AY38910_TYPE_8910 = 0,
    AY38910_TYPE_8912,
    AY38910_TYPE_8913
} ay38910_type_t;

typedef struct {
    ay38910_type_t type;    /* the subtype (default 0 is AY-3-8910) */
    int tick_hz;            /* frequency at which ay38910_tick() will be called in Hz */
    int sound_hz;           /* number of samples that will be produced per second */
    float magnitude;        /* output sample magnitude, from 0.0 (silence) to 1.0 (max volume) */ 
    ay38910_in_t in_cb;     /* I/O port input callback */
    ay38910_out_t out_cb;   /* I/O port output callback */
    void* user_data;        /* optional user-data for callbacks */
} ay38910_desc_t;

typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t bit;
    uint32_t tone_disable;
    uint32_t noise_disable;
} ay38910_tone_t;

typedef struct {
    uint16_t period;
    uint16_t counter;
    uint32_t rng;
    uint32_t bit;
} ay38910_noise_t;

typedef struct {
    uint16_t period;
    uint16_t counter;
    bool shape_holding;
    bool shape_hold;
    uint8_t shape_counter;
    uint8_t shape_state;
} ay38910_env_t;

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

#define AY38910_GET_DATA(p) ((uint8_t)(p>>16))
#define AY38910_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

extern void ay38910_init(ay38910_t* ay, ay38910_desc_t* desc);
extern void ay38910_reset(ay38910_t* ay);
extern uint64_t ay38910_iorq(ay38910_t* ay, uint64_t pins);
extern bool ay38910_tick(ay38910_t* ay);

#ifdef __cplusplus
} /* extern "C" */
#endif
