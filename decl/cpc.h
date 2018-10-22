#pragma once
/*#
    # cpc.h

    An Amstrad CPC emulator in a C header

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
        your own assert macro (default: assert(c))

    You need to include the following headers before including cpc.h:

    - chips/z80.h
    - chips/ay38910.h
    - chips/i8255.h
    - chips/mc6845.h
    - chips/crt.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Amstrad CPC 464

    FIXME!

    ## The Amstrad CPC 6128

    FIXME!

    ## The KC Compact

    FIXME!

    ## TODO

    - improve CRTC emulation, some graphics demos don't work yet
    - DSK file support

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

#define CPC_DISPLAY_WIDTH (768)
#define CPC_DISPLAY_HEIGHT (272)
#define CPC_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define CPC_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */
#define CPC_MAX_TAPE_SIZE (128*1024)        /* max size of tape file in bytes */

/* CPC model types */
typedef enum {
    CPC_TYPE_6128,          /* default */
    CPC_TYPE_464,
    CPC_TYPE_KCCOMPACT
} cpc_type_t;

/* joystick types */
typedef enum {
    CPC_JOYSTICK_NONE,
    CPC_JOYSTICK_DIGITAL,
    CPC_JOYSTICK_ANALOG,
} cpc_joystick_type_t;

/* joystick mask bits */
#define CPC_JOYSTICK_UP    (1<<0)
#define CPC_JOYSTICK_DOWN  (1<<1)
#define CPC_JOYSTICK_LEFT  (1<<2)
#define CPC_JOYSTICK_RIGHT (1<<3)
#define CPC_JOYSTICK_BTN0  (1<<4)
#define CPC_JOYSTICK_BTN1  (1<<4)

/* audio sample data callback */
typedef void (*cpc_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* optional video-decode-debugging callback */
typedef void (*cpc_video_debug_callback_t)(uint64_t crtc_pins, void* user_data);

/* configuration parameters for cpc_init() */
typedef struct {
    cpc_type_t type;                /* default is the CPC 6128 */
    cpc_joystick_type_t joystick_type;

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 1024*312*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for audio- and video-debugging callbacks */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    cpc_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume: 0.0..1.0, default is 0.25 */

    /* an optional callback to generate a video-decode debug visualization */
    cpc_video_debug_callback_t video_debug_cb;

    /* ROM images */
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

/* CPC gate array state */
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

/* CPC emulator state */
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
    /* tape loading */
    int tape_size;      /* tape_size is > 0 if a tape is inserted */
    int tape_pos;
    uint8_t tape_buf[CPC_MAX_TAPE_SIZE];
} cpc_t;

/* initialize a new CPC instance */
extern void cpc_init(cpc_t* cpc, cpc_desc_t* desc);
/* discard a CPC instance */
extern void cpc_discard(cpc_t* cpc);
/* reset a CPC instance */
extern void cpc_reset(cpc_t* cpc);
/* run CPC instance for given amount of micro_seconds */
extern void cpc_exec(cpc_t* cpc, uint32_t micro_seconds);
/* send a key down event */
extern void cpc_key_down(cpc_t* cpc, int key_code);
/* send a key up event */
extern void cpc_key_up(cpc_t* cpc, int key_code);
/* enable/disable joystick emulation */
extern void cpc_set_joystick_type(cpc_t* sys, cpc_joystick_type_t type);
/* get current joystick emulation type */
extern cpc_joystick_type_t cpc_joystick_type(cpc_t* sys);
/* set joystick mask (combination of CPC_JOYSTICK_*) */
extern void cpc_joystick(cpc_t* sys, uint8_t mask);
/* load a snapshot file (.sna or .bin) into the emulator */
extern bool cpc_quickload(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* insert a tape file (.tap) */
extern bool cpc_insert_tape(cpc_t* cpc, const uint8_t* ptr, int num_bytes);
/* remove currently inserted tape */
extern void cpc_remove_tape(cpc_t* cpc);
/* if enabled, start calling the video-debugging-callback */
extern void cpc_enable_video_debugging(cpc_t* cpc, bool enabled);
/* get current display debug visualization enabled/disabled state */
extern bool cpc_video_debugging_enabled(cpc_t* cpc);
/* low-level pixel decoding, this is public as support for video debugging callbacks */
extern void cpc_ga_decode_pixels(cpc_t* sys, uint32_t* dst, uint64_t crtc_pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
