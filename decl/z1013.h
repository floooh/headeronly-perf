#pragma once
/*#
    # z1013.h

    A Robotron Z1013 emulator in a C header.

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

    You need to include the following headers before including z1013.h:

    - chips/z80.h
    - chips/z80pio.h
    - chips/mem.h
    - chips/kbd.h
    - chips/clk.h

    ## The Robotron Z1013

    The Z1013 was a very simple East German home computer, basically
    just a Z80 CPU connected to some memory, and a PIO connected to
    a keyboard matrix. It's easy to emulate because the system didn't
    use any interrupts, and only simple PIO IN/OUT is required to
    scan the keyboard matrix.

    It had a 32x32 monochrome ASCII framebuffer starting at EC00,
    and a 2 KByte operating system ROM starting at F000.

    No cassette-tape / beeper sound emulated!

    ## TODO: add hardware/software reference links

    ## TODO: Describe Usage


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

/* the width and height of the Z1013 display in pixels */
#define Z1013_DISPLAY_WIDTH (256)
#define Z1013_DISPLAY_HEIGHT (256)

/* Z1013 model types */
typedef enum {
    Z1013_TYPE_64,      /* Z1013.64 (default, latest model with 2 MHz and 64 KB RAM, new ROM) */
    Z1013_TYPE_16,      /* Z1013.16 (2 MHz model with 16 KB RAM, new ROM) */
    Z1013_TYPE_01,      /* Z1013.01 (original model, 1 MHz, 16 KB RAM) */
} z1013_type_t;

/* configuration parameters for z1013_setup() */
typedef struct {
    z1013_type_t type;          /* default is Z1013_TYPE_64 */

    /* video output config */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 256*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* ROM images */
    const void* rom_mon202;
    const void* rom_mon_a2;
    const void* rom_font;
    int rom_mon202_size;
    int rom_mon_a2_size;
    int rom_font_size;
} z1013_desc_t;

/* Z1013 emulator state */
typedef struct {
    z80_t cpu;
    z80pio_t pio;
    bool valid;
    z1013_type_t type;
    uint8_t kbd_request_column;
    bool kbd_request_line_hilo;
    uint32_t* pixel_buffer;
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    uint8_t ram[1<<16];
    uint8_t rom_os[2048];
    uint8_t rom_font[2048];
} z1013_t;

/* initialize a new Z1013 instance */
extern void z1013_init(z1013_t* sys, const z1013_desc_t* desc);
/* discard a z1013 instance */
extern void z1013_discard(z1013_t* sys);
/* reset Z1013 instance */
extern void z1013_reset(z1013_t* sys);
/* run the Z1013 instance for a given number of microseconds */
extern void z1013_exec(z1013_t* sys, uint32_t micro_seconds);
/* send a key-down event */
extern void z1013_key_down(z1013_t* sys, int key_code);
/* send a key-up event */
extern void z1013_key_up(z1013_t* sys, int key_code);
/* load a "KC .z80" file into the emulator */
extern bool z1013_quickload(z1013_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
