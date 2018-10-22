#pragma once
/*#
    # z9001.h

    A Robotron Z9001/KC87 emulator in a C header.

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

    You need to include the following headers before including z9001.h:

    - chips/z80.h
    - chips/z80pio.h
    - chips/z80ctc.h
    - chips/beeper.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h
  
    ## The Robotron Z9001

    The Z9001 (later retconned to KC85/1) was independently developed
    to the HC900 (aka KC85/2) by Robotron Dresden. It had a pretty
    slick design with an integrated keyboard which was legendary for
    how hard it was to type on.\n\nThe standard model had 16 KByte RAM,
    a monochrome 40x24 character display, and a 2.5 MHz U880 CPU
    (making it the fastest East German 8-bitter). The Z9001 could
    be extended by up to 4 expansion modules, usually one or two
    16 KByte RAM modules, and a 10 KByte ROM BASIC module (the
    version emulated here comes with 32 KByte RAM and a BASIC module).

    ## The Robotron KC87

    The KC87 was the successor to the KC85/1. The only real difference
    was the built-in BASIC interpreter in ROM. The KC87 emulated here
    has 48 KByte RAM and the video color extension which could
    assign 8 foreground and 8 (identical) background colors per character,
    plus a blinking flag. This video extension was already available on the
    Z9001 though.

    ## TODO:
    - enable/disable audio on PIO1-A bit 7
    - border color
    - 40x20 video mode

    ## Reference Info

    - schematics: http://www.sax.de/~zander/kc/kcsch_1.pdf
    - manual: http://www.sax.de/~zander/z9001/doku/z9_fub.pdf

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

#define Z9001_DISPLAY_WIDTH (320)   /* display width in pixels */
#define Z9001_DISPLAY_HEIGHT (192)  /* display height in pixels */
#define Z9001_MAX_AUDIO_SAMPLES (1024)      /* max number of audio samples in internal sample buffer */
#define Z9001_DEFAULT_AUDIO_SAMPLES (128)   /* default number of samples in internal sample buffer */ 

/* Z9001/KC87 model types */
typedef enum {
    Z9001_TYPE_Z9001,   /* the original Z9001 (default) */
    Z9001_TYPE_KC87,    /* the revised KC87 with built-in BASIC and color module */
} z9001_type_t;

/* Z9001 audio sample data callback */
typedef void (*z9001_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for z9001_init() */
typedef struct {
    z9001_type_t type;                  /* default is Z9001_TYPE_Z9001 */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*192*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user data for call back functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    z9001_audio_callback_t audio_cb;    /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is Z9001_DEFAULT_AUDIO_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* volume of generated audio: 0.0..1.0, default is 0.5 */

    /* ROMs for Z9001 */
    const void* rom_z9001_os_1;
    const void* rom_z9001_os_2;
    const void* rom_z9001_font;
    int rom_z9001_os_1_size;
    int rom_z9001_os_2_size;
    int rom_z9001_font_size;

    /* optional BASIC module for z9001 */
    const void* rom_z9001_basic;
    int rom_z9001_basic_size;

    /* ROMs for KC87 */
    const void* rom_kc87_os;
    const void* rom_kc87_basic;
    const void* rom_kc87_font;
    int rom_kc87_os_size;
    int rom_kc87_basic_size;
    int rom_kc87_font_size;
} z9001_desc_t;

/* Z9001 emulator state */
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
    /* FIXME: uint8_t border_color; */
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

/* initialize a new Z9001 instance */
extern void z9001_init(z9001_t* sys, const z9001_desc_t* desc);
/* discard a Z9001 instance */
extern void z9001_discard(z9001_t* sys);
/* reset Z9001 instance */
extern void z9001_reset(z9001_t* sys);
/* run Z9001 instance for a given number of microseconds */
extern void z9001_exec(z9001_t* sys, uint32_t micro_seconds);
/* send a key-down event */
extern void z9001_key_down(z9001_t* sys, int key_code);
/* send a key-up event */
extern void z9001_key_up(z9001_t* sys, int key_code);
/* load a KC TAP or KCC file into the emulator */
extern bool z9001_quickload(z9001_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
