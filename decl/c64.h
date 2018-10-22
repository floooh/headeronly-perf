#pragma once
/*#
    # 64.h

    An Commodore C64 (PAL) emulator in a C header

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

    - chips/m6502.h
    - chips/m6526.h
    - chips/m6569.h
    - chips/m6581.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The Commodore C64

    TODO!

    ## TODO:

    - emulate separate joystick 1 and 2
    - improve game fast loader compatibility
    - floppy disc support

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

#define C64_DISPLAY_WIDTH (392)             /* required framebuffer width in pixels */
#define C64_DISPLAY_HEIGHT (272)            /* required framebuffer hight in pixels */
#define C64_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define C64_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */ 
#define C64_MAX_TAPE_SIZE (512*1024)        /* max size of cassette tape image */

/* C64 joystick types */
typedef enum {
    C64_JOYSTICKTYPE_NONE,
    C64_JOYSTICKTYPE_DIGITAL_1,
    C64_JOYSTICKTYPE_DIGITAL_2,
    C64_JOYSTICKTYPE_PADDLE_1,  /* FIXME: not emulated */
    C64_JOYSTICKTYPE_PADDLE_2,  /* FIXME: not emulated */
} c64_joystick_type_t;

/* joystick mask bits */
#define C64_JOYSTICK_UP    (1<<0)
#define C64_JOYSTICK_DOWN  (1<<1)
#define C64_JOYSTICK_LEFT  (1<<2)
#define C64_JOYSTICK_RIGHT (1<<3)
#define C64_JOYSTICK_BTN   (1<<4)

/* audio sample data callback */
typedef void (*c64_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* config parameters for c64_init() */
typedef struct {
    c64_joystick_type_t joystick_type;  /* default is C64_JOYSTICK_NONE */

    /* video output config (if you don't want video decoding, set these to 0) */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 392*272*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    c64_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is C64_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_sid_volume;         /* audio volume of the SID chip (0.0 .. 1.0), default is 1.0 */
    float audio_beeper_volume;      /* audio volume of the tape-beeper (0.0 .. 1.0), default is 0.1 */
    bool audio_tape_sound;          /* if true, tape loading is audible */

    /* ROM images */
    const void* rom_char;           /* 4 KByte character ROM dump */
    const void* rom_basic;          /* 8 KByte BASIC dump */
    const void* rom_kernal;         /* 8 KByte KERNAL dump */
    int rom_char_size;
    int rom_basic_size;
    int rom_kernal_size;
} c64_desc_t;

/* C64 emulator state */
typedef struct {
    m6502_t cpu;
    m6526_t cia_1;
    m6526_t cia_2;
    m6569_t vic;
    m6581_t sid;
    beeper_t beeper;            /* used for the tape-sound */
    
    bool valid;
    c64_joystick_type_t joystick_type;
    uint8_t joystick_active;
    bool io_mapped;             /* true when D000..DFFF is has IO area mapped in */
    uint8_t cpu_port;           /* last state of CPU port (for memory mapping) */
    uint8_t kbd_joy1_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t kbd_joy2_mask;      /* current joystick-2 state from keyboard-joystick emulation */
    uint8_t joy_joy1_mask;      /* current joystick-1 state from c64_joystick() */
    uint8_t joy_joy2_mask;      /* current joystick-2 state from c64_joystick() */
    uint16_t vic_bank_select;   /* upper 4 address bits from CIA-2 port A */

    clk_t clk;
    kbd_t kbd;
    mem_t mem_cpu;
    mem_t mem_vic;

    void* user_data;
    uint32_t* pixel_buffer;
    c64_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[C64_MAX_AUDIO_SAMPLES];

    uint8_t color_ram[1024];        /* special static color ram */
    uint8_t ram[1<<16];             /* general ram */
    uint8_t rom_char[0x1000];       /* 4 KB character ROM image */
    uint8_t rom_basic[0x2000];      /* 8 KB BASIC ROM image */
    uint8_t rom_kernal[0x2000];     /* 8 KB KERNAL V3 ROM image */

    bool tape_motor;    /* tape motor on/off */
    bool tape_button;   /* play button on tape pressed/unpressed */
    bool tape_sound;    /* true when tape is audible */
    int tape_size;      /* tape_size > 0: a tape is inserted */
    int tape_pos;        
    int tape_tick_count;
    uint8_t tape_buf[C64_MAX_TAPE_SIZE];
} c64_t;

/* initialize a new C64 instance */
extern void c64_init(c64_t* sys, const c64_desc_t* desc);
/* discard C64 instance */
extern void c64_discard(c64_t* sys);
/* reset a C64 instance */
extern void c64_reset(c64_t* sys);
/* tick C64 instance for a given number of microseconds */
extern void c64_exec(c64_t* sys, uint32_t micro_seconds);
/* send a key-down event to the C64 */
extern void c64_key_down(c64_t* sys, int key_code);
/* send a key-up event to the C64 */
extern void c64_key_up(c64_t* sys, int key_code);
/* enable/disable joystick emulation */
extern void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type);
/* get current joystick emulation type */
extern c64_joystick_type_t c64_joystick_type(c64_t* sys);
/* set joystick mask (combination of C64_JOYSTICK_*) */
extern void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask);
/* insert a tape file */
extern bool c64_insert_tape(c64_t* sys, const uint8_t* ptr, int num_bytes);
/* remove tape file */
extern void c64_remove_tape(c64_t* sys);
/* start the tape (press the Play button) */
extern void c64_start_tape(c64_t* sys);
/* stop the tape (unpress the Play button */
extern void c64_stop_tape(c64_t* sys);
/* quickload a .bin file (only tested with wlorenz tests) */
extern bool c64_quickload(c64_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
