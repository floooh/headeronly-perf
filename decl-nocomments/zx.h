#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ZX_DISPLAY_WIDTH (320)   /* display width in pixels */
#define ZX_DISPLAY_HEIGHT (256)  /* display height in pixels */
#define ZX_MAX_AUDIO_SAMPLES (1024)      /* max number of audio samples in internal sample buffer */
#define ZX_DEFAULT_AUDIO_SAMPLES (128)   /* default number of samples in internal sample buffer */ 

typedef enum {
    ZX_TYPE_48K,
    ZX_TYPE_128,
} zx_type_t;

typedef enum {
    ZX_JOYSTICKTYPE_NONE,
    ZX_JOYSTICKTYPE_KEMPSTON,
    ZX_JOYSTICKTYPE_SINCLAIR_1,
    ZX_JOYSTICKTYPE_SINCLAIR_2,
} zx_joystick_type_t;

#define ZX_JOYSTICK_RIGHT   (1<<0)
#define ZX_JOYSTICK_LEFT    (1<<1)
#define ZX_JOYSTICK_DOWN    (1<<2)
#define ZX_JOYSTICK_UP      (1<<3)
#define ZX_JOYSTICK_BTN     (1<<4)

typedef void (*zx_audio_callback_t)(const float* samples, int num_samples, void* user_data);

typedef struct {
    zx_type_t type;                     /* default is ZX_TYPE_48K */
    zx_joystick_type_t joystick_type;   /* what joystick to emulate, default is ZX_JOYSTICK_NONE */

    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    void* user_data;

    zx_audio_callback_t audio_cb;   /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_beeper_volume;      /* volume of the ZX48K beeper: 0.0..1.0, default is 0.25 */
    float audio_ay_volume;          /* volume of the ZX128 AY sound chip: 0.0..1.0, default is 0.5 */

    const void* rom_zx48k;
    int rom_zx48k_size;

    const void* rom_zx128_0;
    const void* rom_zx128_1;
    int rom_zx128_0_size;
    int rom_zx128_1_size;
} zx_desc_t;

typedef struct {
    z80_t cpu;
    beeper_t beeper;
    ay38910_t ay;
    bool valid;
    zx_type_t type;
    zx_joystick_type_t joystick_type;
    bool memory_paging_disabled;
    uint8_t kbd_joymask;        /* joystick mask from keyboard joystick emulation */
    uint8_t joy_joymask;        /* joystick mask from zx_joystick() */
    uint32_t tick_count;
    uint8_t last_fe_out;            /* last out value to 0xFE port */
    uint8_t blink_counter;          /* incremented on each vblank */
    int frame_scan_lines;
    int top_border_scanlines;
    int scanline_period;
    int scanline_counter;
    int scanline_y;
    uint32_t display_ram_bank;
    uint32_t border_color;
    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    uint32_t* pixel_buffer;
    void* user_data;
    zx_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[ZX_MAX_AUDIO_SAMPLES];
    uint8_t ram[8][0x4000];
    uint8_t rom[2][0x4000];
    uint8_t junk[0x4000];
} zx_t;

extern void zx_init(zx_t* sys, const zx_desc_t* desc);
extern void zx_discard(zx_t* sys);
extern void zx_reset(zx_t* sys);
extern void zx_exec(zx_t* sys, uint32_t micro_seconds);
extern void zx_key_down(zx_t* sys, int key_code);
extern void zx_key_up(zx_t* sys, int key_code);
extern void zx_set_joystick_type(zx_t* sys, zx_joystick_type_t type);
extern zx_joystick_type_t zx_joystick_type(zx_t* sys);
extern void zx_joystick(zx_t* sys, uint8_t mask);
extern bool zx_quickload(zx_t* sys, const uint8_t* ptr, int num_bytes); 

#ifdef __cplusplus
} /* extern "C" */
#endif
