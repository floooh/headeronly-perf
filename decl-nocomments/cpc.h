#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CPC_DISPLAY_WIDTH (768)
#define CPC_DISPLAY_HEIGHT (272)
#define CPC_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define CPC_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */
#define CPC_MAX_TAPE_SIZE (128*1024)        /* max size of tape file in bytes */

typedef enum {
    CPC_TYPE_6128,          /* default */
    CPC_TYPE_464,
    CPC_TYPE_KCCOMPACT
} cpc_type_t;

typedef enum {
    CPC_JOYSTICK_NONE,
    CPC_JOYSTICK_DIGITAL,
    CPC_JOYSTICK_ANALOG,
} cpc_joystick_type_t;

#define CPC_JOYSTICK_UP    (1<<0)
#define CPC_JOYSTICK_DOWN  (1<<1)
#define CPC_JOYSTICK_LEFT  (1<<2)
#define CPC_JOYSTICK_RIGHT (1<<3)
#define CPC_JOYSTICK_BTN0  (1<<4)
#define CPC_JOYSTICK_BTN1  (1<<4)

typedef void (*cpc_audio_callback_t)(const float* samples, int num_samples, void* user_data);

typedef void (*cpc_video_debug_callback_t)(uint64_t crtc_pins, void* user_data);

typedef struct {
    cpc_type_t type;                /* default is the CPC 6128 */
    cpc_joystick_type_t joystick_type;

    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 1024*312*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    void* user_data;

    cpc_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume: 0.0..1.0, default is 0.25 */

    cpc_video_debug_callback_t video_debug_cb;

    const void* rom_464_os;
    const void* rom_464_basic;
    const void* rom_6128_os;
    const void* rom_6128_basic;
    const void* rom_6128_amsdos;
    const void* rom_kcc_os;
    const void* rom_kcc_basic;
    int rom_464_os_size;
    int rom_464_basic_size;
    int rom_6128_os_size;
    int rom_6128_basic_size;
    int rom_6128_amsdos_size;
    int rom_kcc_os_size;
    int rom_kcc_basic_size;
} cpc_desc_t;

typedef struct {
    uint8_t config;                 /* out to port 0x7Fxx func 0x80 */
    uint8_t next_video_mode;
    uint8_t video_mode;
    uint8_t ram_config;             /* out to port 0x7Fxx func 0xC0 */
    uint8_t pen;                    /* currently selected pen (or border) */
    uint32_t colors[32];            /* CPC and KC Compact have slightly different colors */
    uint32_t palette[16];           /* the current pen colors */
    uint32_t border_color;          /* the current border color */
    int hsync_irq_counter;          /* incremented each scanline, reset at 52 */
    int hsync_after_vsync_counter;   /* for 2-hsync-delay after vsync */
    int hsync_delay_counter;        /* hsync to monitor is delayed 2 ticks */
    int hsync_counter;              /* countdown until hsync to monitor is deactivated */
    bool sync;                      /* gate-array generated video sync (modified HSYNC) */
    bool intr;                      /* GA interrupt pin active */
    uint64_t crtc_pins;             /* store CRTC pins to detect rising/falling bits */
} cpc_gatearray_t;

typedef struct {
    z80_t cpu;
    ay38910_t psg;
    mc6845_t vdg;
    i8255_t ppi;

    bool valid;
    bool dbgvis;                    /* debug visualzation enabled? */
    cpc_type_t type;
    cpc_joystick_type_t joystick_type;
    uint8_t kbd_joymask;
    uint8_t joy_joymask;
    uint8_t upper_rom_select;
    uint32_t tick_count;
    uint16_t casread_trap;
    uint16_t casread_ret;
    cpc_gatearray_t ga;

    crt_t crt;
    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    uint32_t* pixel_buffer;
    void* user_data;
    cpc_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[CPC_MAX_AUDIO_SAMPLES];
    bool video_debug_enabled;
    cpc_video_debug_callback_t video_debug_cb;
    uint8_t ram[8][0x4000];
    uint8_t rom_os[0x4000];
    uint8_t rom_basic[0x4000];
    uint8_t rom_amsdos[0x4000];
    int tape_size;      /* tape_size is > 0 if a tape is inserted */
    int tape_pos;
    uint8_t tape_buf[CPC_MAX_TAPE_SIZE];
} cpc_t;

extern void cpc_init(cpc_t* cpc, cpc_desc_t* desc);
extern void cpc_discard(cpc_t* cpc);
extern void cpc_reset(cpc_t* cpc);
extern void cpc_exec(cpc_t* cpc, uint32_t micro_seconds);
extern void cpc_key_down(cpc_t* cpc, int key_code);
extern void cpc_key_up(cpc_t* cpc, int key_code);
extern void cpc_set_joystick_type(cpc_t* sys, cpc_joystick_type_t type);
extern cpc_joystick_type_t cpc_joystick_type(cpc_t* sys);
extern void cpc_joystick(cpc_t* sys, uint8_t mask);
extern bool cpc_quickload(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
extern bool cpc_insert_tape(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
extern void cpc_remove_tape(cpc_t* cpc);
extern void cpc_enable_video_debugging(cpc_t* cpc, bool enabled);
extern bool cpc_video_debugging_enabled(cpc_t* cpc);
extern void cpc_ga_decode_pixels(cpc_t* sys, uint32_t* dst, uint64_t crtc_pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
