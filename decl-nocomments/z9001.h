#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z9001_DISPLAY_WIDTH (320)   /* display width in pixels */
#define Z9001_DISPLAY_HEIGHT (192)  /* display height in pixels */
#define Z9001_MAX_AUDIO_SAMPLES (1024)      /* max number of audio samples in internal sample buffer */
#define Z9001_DEFAULT_AUDIO_SAMPLES (128)   /* default number of samples in internal sample buffer */ 

typedef enum {
    Z9001_TYPE_Z9001,   /* the original Z9001 (default) */
    Z9001_TYPE_KC87,    /* the revised KC87 with built-in BASIC and color module */
} z9001_type_t;

typedef void (*z9001_audio_callback_t)(const float* samples, int num_samples, void* user_data);

typedef struct {
    z9001_type_t type;                  /* default is Z9001_TYPE_Z9001 */

    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*192*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    void* user_data;

    z9001_audio_callback_t audio_cb;    /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is Z9001_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* volume of generated audio: 0.0..1.0, default is 0.5 */

    const void* rom_z9001_os_1;
    const void* rom_z9001_os_2;
    const void* rom_z9001_font;
    int rom_z9001_os_1_size;
    int rom_z9001_os_2_size;
    int rom_z9001_font_size;

    const void* rom_z9001_basic;
    int rom_z9001_basic_size;

    const void* rom_kc87_os;
    const void* rom_kc87_basic;
    const void* rom_kc87_font;
    int rom_kc87_os_size;
    int rom_kc87_basic_size;
    int rom_kc87_font_size;
} z9001_desc_t;

typedef struct {
    z80_t cpu;
    z80pio_t pio1;
    z80pio_t pio2;
    z80ctc_t ctc;
    beeper_t beeper;
    bool valid;
    z9001_type_t type;
    uint64_t ctc_zcto2;     /* pin mask to store state of CTC ZCTO2 */
    uint32_t blink_counter;
    bool blink_flip_flop;
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    uint32_t* pixel_buffer;
    void* user_data;
    z9001_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[Z9001_MAX_AUDIO_SAMPLES];
    uint8_t ram[1<<16];
    uint8_t rom[0x4000];
    uint8_t rom_font[0x0800];   /* 2 KB font ROM (not mapped into CPU address space) */
} z9001_t;

extern void z9001_init(z9001_t* sys, const z9001_desc_t* desc);
extern void z9001_discard(z9001_t* sys);
extern void z9001_reset(z9001_t* sys);
extern void z9001_exec(z9001_t* sys, uint32_t micro_seconds);
extern void z9001_key_down(z9001_t* sys, int key_code);
extern void z9001_key_up(z9001_t* sys, int key_code);
extern bool z9001_quickload(z9001_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
