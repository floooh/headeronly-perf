#pragma once
/*#
    # atom.h

    Acorn Atom emulator in a C header.

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

    You need to include the following headers before including atom.h:

    - chips/m6502.h
    - chips/mc6847.h
    - chips/i8255.h
    - chips/m6522.h
    - chips/beeper.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Acorn Atom

    FIXME!

    ## TODO

    - VIA emulation is currently only minimal
    - handle shift key (some games use this as jump button)
    - AtomMMC is very incomplete (only what's needed for joystick)

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

#define ATOM_DISPLAY_WIDTH (MC6847_DISPLAY_WIDTH)
#define ATOM_DISPLAY_HEIGHT (MC6847_DISPLAY_HEIGHT)
#define ATOM_MAX_AUDIO_SAMPLES (1024)       /* max number of audio samples in internal sample buffer */
#define ATOM_DEFAULT_AUDIO_SAMPLES (128)    /* default number of samples in internal sample buffer */
#define ATOM_MAX_TAPE_SIZE (1<<16)          /* max size of tape file in bytes */

/* joystick emulation types */
typedef enum {
    ATOM_JOYSTICKTYPE_NONE,
    ATOM_JOYSTICKTYPE_MMC
} atom_joystick_type_t;

/* joystick mask bits */
#define ATOM_JOYSTICK_RIGHT (1<<0)
#define ATOM_JOYSTICK_LEFT  (1<<1)
#define ATOM_JOYSTICK_DOWN  (1<<2)
#define ATOM_JOYSTICK_UP    (1<<3)
#define ATOM_JOYSTICK_BTN   (1<<4)

/* audio sample data callback */
typedef void (*atom_audio_callback_t)(const float* samples, int num_samples, void* user_data);

/* configuration parameters for atom_init() */
typedef struct {
    atom_joystick_type_t joystick_type;     /* what joystick type to emulate, default is ATOM_JOYSTICK_NONE */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callbacks */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    atom_audio_callback_t audio_cb;   /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is ZX_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_volume;             /* audio volume: 0.0..1.0, default is 0.25 */

    /* ROM images */
    const void* rom_abasic;
    const void* rom_afloat;
    const void* rom_dosrom;
    int rom_abasic_size;
    int rom_afloat_size;
    int rom_dosrom_size;
} atom_desc_t;

/* Acorn Atom emulation state */
typedef struct {
    m6502_t cpu;
    mc6847_t vdg;
    i8255_t ppi;
    m6522_t via;
    beeper_t beeper;
    bool valid;
    int counter_2_4khz;
    int period_2_4khz;
    bool state_2_4khz;
    bool out_cass0;
    bool out_cass1;
    atom_joystick_type_t joystick_type;
    uint8_t kbd_joymask;        /* joystick mask from keyboard-joystick-emulation */
    uint8_t joy_joymask;        /* joystick mask from calls to atom_joystick() */
    uint8_t mmc_cmd;
    uint8_t mmc_latch;
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    void* user_data;
    atom_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[ATOM_MAX_AUDIO_SAMPLES];
    uint8_t ram[0xA000];
    uint8_t rom_abasic[0x2000];
    uint8_t rom_afloat[0x1000];
    uint8_t rom_dosrom[0x1000];
    /* tape loading */
    int tape_size;  /* tape_size is > 0 if a tape is inserted */
    int tape_pos;
    uint8_t tape_buf[ATOM_MAX_TAPE_SIZE];
} atom_t;

/* initialize a new Atom instance */
extern void atom_init(atom_t* sys, const atom_desc_t* desc);
/* discard Atom instance */
extern void atom_discard(atom_t* sys);
/* reset Atom instance */
extern void atom_reset(atom_t* sys);
/* run Atom instance for a number of microseconds */
extern void atom_exec(atom_t* sys, uint32_t micro_seconds);
/* send a key down event */
extern void atom_key_down(atom_t* sys, int key_code);
/* send a key up event */
extern void atom_key_up(atom_t* sys, int key_code);
/* enable/disable joystick emulation */
extern void atom_set_joystick_type(atom_t* sys, atom_joystick_type_t type);
/* get current joystick emulation type */
extern atom_joystick_type_t atom_joystick_type(atom_t* sys);
/* set joystick mask (combination of ATOM_JOYSTICK_*) */
extern void atom_joystick(atom_t* sys, uint8_t mask);
/* insert a tape for loading (must be an Atom TAP file), data will be copied */
extern bool atom_insert_tape(atom_t* sys, const uint8_t* ptr, int num_bytes);
/* remove tape */
extern void atom_remove_tape(atom_t* sys);

#ifdef __cplusplus
} /* extern "C" */
#endif
