#pragma once
/*#
    # kc85.h

    A KC85/2, /3 and /4 emulator in a C header.

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

    You need to include the following headers before including kc85.h:

    - chips/z80.h
    - chips/z80ctc.h
    - chips/z80pio.h
    - chips/beeper.h
    - chips/kbd.h
    - chips/mem.h
    - chips/clk.h

    ## The KC85/2

    This was the ur-model of the KC85 family designed and manufactured
    by VEB Mikroelekronikkombinat Muehlhausen. The KC85/2 was introduced
    in 1984 as HC-900, and renamed to KC85/2 in 1985 (at the same time
    when the completely unrelated Z9001 was renamed to KC85/1).

        - U880 CPU @ 1.75 MHz (the U880 was an "unlicensed" East German Z80 clone)
        - 1x U855 (clone of the Z80-PIO)
        - 1x U857 (clone of the Z80-CTC)
        - 16 KByte RAM at 0000..3FFF
        - 16 KByte video RAM (IRM) at 8000..BFFF
        - 4 KByte ROM in 2 sections (E000..E800 and F000..F800)
        - the operating system was called CAOS (Cassette Aided Operatin System)
        - 50 Hz PAL video at 320x256 pixels
        - Speccy-like color attributes (1 color byte per 8x4 pixels)
        - fixed palette of 16 foreground and 8 background colors
        - square-wave-beeper sound
        - separate keyboard with a serial-encoder chip to transfer
          key strokes to the main unit
        - flexible expansion module system (2 slots in the base units,
          4 additional slots per 'BUSDRIVER' units)
        - a famously bizarre video memory layout, consisting of a
          256x256 chunk on the left, and a separate 64x256 chunk on the right,
          with vertically 'interleaved' vertical addressing similar to the
          ZX Spectrum but with different offsets

    ### Memory Map:
        - 0000..01FF:   OS variables, interrupt vectors, and stack
        - 0200..3FFF:   usable RAM
        - 8000..A7FF:   pixel video RAM (1 byte => 8 pixels)
        - A800..B1FF:   color video RAM (1 byte => 8x4 color attribute block)
        - B200..B6FF:   ASCII backing buffer
        - B700..B77F:   cassette tape buffer
        - B780..B8FF:   more OS variables
        - B800..B8FF:   backing buffer for expansion module control bytes
        - B900..B97F:   buffer for actions assigned to function keys
        - B980..B9FF:   window attributes buffers
        - BA00..BBFF:   "additional programs"
        - BC00..BFFF:   usable 'slow-RAM'
        - E000..E7FF:   2 KB ROM
        - F000..F7FF:   2 KB ROM

        The video memory from A000..BFFF has slow CPU access (2.4us) because
        it needs to share memory accesses with the video system. Also, CPU
        accesses to this RAM block are visible as 'display needling' artefacts.

        Both video memory effects are not currently emulated.

    ### Special Operating System Conditions

        - the index register IX is reserved for operating system use
          and must not be changed while interrupts are enabled
        - only interrupt mode IM2 is supported
    
    ### Interrupt Vectors:
        - 01E4:     PIO-A (cassette tape input)
        - 01E6:     PIO-B (keyboard input)
        - 01E8:     CTC-0 (free)
        - 01EA:     CTC-1 (cassette tape output)
        - 01EC:     CTC-2 (timer interrupt used for sound length)

    ## IO Port Map: 
        - 80:   Expansion module control (OUT: write module control byte,
                IN: read module id in slot). The upper 8 bits on the 
                address bus identify the module slot (in the base 
                unit the two slot addresses are 08 and 0C).
        - 88:   PIO port A, data
        - 89:   PIO port B, data
        - 8A:   PIO port A, control
        - 8B:   PIO port B, control
        - 8C:   CTC channel 0
        - 8D:   CTC channel 1
        - 8E:   CTC channel 2
        - 8F:   CTC channel 3
        
        The PIO port A and B bits are used to control bank switching and
        other hardware features:

        - PIO-A:
            - bit 0:    switch ROM at E000..FFFF on/off
            - bit 1:    switch RAM at 0000..3FFF on/off
            - bit 2:    switch video RAM (IRM) at 8000..BFFF on/off
            - bit 3:    write-protect RAM at 0000
            - bit 4:    unused
            - bit 5:    switch the front-plate LED on/off
            - bit 6:    cassette tape player motor control
            - bit 7:    expansion ROM at C000 on/off
        - PIO-B:
            - bits 0..4:    sound volume (currently not implemented)
            - bits 5/6:     unused
            - bit 7:        enable/disable the foreground-color blinking

        The CTC channels are used for sound frequency and other timing tasks:

        - CTC-0:    sound output (left?)
        - CTC-1:    sound output (right?)
        - CTC-2:    foreground color blink frequency, timer for cassette input
        - CTC-3:    timer for keyboard input
            
    ## The Module System:

    This emulator supports the most common RAM- and ROM-modules,
    but doesn't emulate special-hardware modules like the V24 or 
    A/D converter module.

    The module system works with 4 byte values:

    - The **slot address**, the two base unit slots are at address 08 and 0C
    - The **module id**, this is a fixed value that identifies a module type,
      most notable, all 16 KByte ROM application modules had the same id,
      the module id can be queried by reading from port 80, with the
      slot address in the upper 8 bit of the 16-bit port address (so 
      to query what module is in slot C, you would do an IN A,(C),
      with the value 0C80 in BC). If no module is in the slot, the value
      FF would be written to A, otherwise the module's id byte
    - The module's **address mask**, this is a byte value that's ANDed
      against the upper 8 address bytes when mapping the module to memory,
      this essentially clamps a module's address to a 'round' 8- or
      16 KByte value (these are the 2 values I've seen in the wild)
    - The module control byte, this controls whether a module is currently
      activate (bit 0), write-protected (bit 1), and at what address the 
      module is mapped into the 16-bit address space (upper 3 bits)

    The module system is controlled with the SWITCH command, for instance
    the following command would map a ROM module in slot 8 to address
    C000:

        SWITCH 8 C1

    A RAM module in slot 0C mapped to address 4000:

        SWITCH C 43

    If you want to write-protect the RAM:

        SWITCH C 41

    ## The KC85/3

    The KC85/3 had the same hardware as the KC85/2 but came with a builtin
    8 KByte BASIC ROM at address C000..DFFF, and the OS was bumped to 
    CAOS 3.1, now taking up a full 8 KBytes. Despite being just a minor
    update to the KC85/2, the KC85/3 was (most likely) the most popular
    model of the KC85/2 family.

    ## The KC85/4

    The KC85/4 was a major upgrade to the KC85/2 hardware architecture:

    - 64 KByte usable RAM
    - 64 KByte video RAM split up into 4 16-KByte banks
    - 20 KByte ROM (8 KByte BASIC, and 8+4 KByte OS)
    - Improved color attribute resolution (8x1 pixels instead of 8x4)
    - An additional per-pixel color mode which allowed to assign each
      individual pixel one of 4 hardwired colors at full 320x256
      resolution, this was realized by using 1 bit from the 
      pixel-bank and the other bit from the color-bank, so setting
      one pixel required 2 memory accesses and a bank switch. Maybe
      this was the reason why this mode was hardly used (this
      per-pixel-color mode is currently not implemented in this emulator).
    - Improved '90-degree-rotated' video memory layout, the 320x256
      pixel video memory was organized as 40 vertical stacks of 256 bytes,
      and the entire video memory was linear, this was perfectly suited
      to the Z80's 8+8 bit register pairs. The upper 8-bit register 
      (for instance H) would hold the 'x coordinate' (columns 0 to 39),
      and the lower 8-bit register (L) the y coordinate (lines 0 to 255).
    - 64 KByte video memory was organized into 4 16-KByte banks, 2 banks
      for pixels, and 2 banks for colors. One pixel+color bank pair could
      be displayed while the other could be accessed by the CPU, this enabled
      true hardware double-buffering (unfortunately everything else was
      hardwired, so things like hardware-scrolling were not possible).

    The additional memory bank switching options were realized through
    previously unused bits in the PIO A/B ports, and 2 additional
    write-only 8-bit latches at port address 84 and 86:

    New bits in PIO port B:
        - bit 5:    enable the 2 stacked RAM bank at address 8000
        - bit 6:    write protect RAM bank at address 8000 

    Output port 84:
        - bit 0:    select  the pixel/color bank pair 0 or 1 for display
        - bit 1:    select the pixel (0) or color bank (1) for CPU access
        - bit 2:    select the pixel/color bank pair 0 or 1 for CPU access
        - bit 3:    active the per-pixel-color-mode
        - bit 4:    select one of two RAM banks at address 8000
        - bit 5:    ??? (the docs say "RAM Block Select Bit for RAM8")
        - bits 6/7: unused

    Output port 86:
        - bit 0:        enable the 16K RAM bank at address 4000
        - bit 1:        write-protection for for RAM bank at address 4000
        - bits 2..6:    unused
        - bit 7:        enable the 4 KByte CAOS ROM bank at C000

    ## TODO:

    - optionally proper keyboard emulation (the current implementation
      uses a shortcut to directly write the key code into a memory address)
    - video-decoding is currently per-scanline
    - KC85/4 pixel-color mode
    - wait states for video RAM access
    - display needling
    - audio volume is currently not implemented

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

#define KC85_DISPLAY_WIDTH (320)
#define KC85_DISPLAY_HEIGHT (256)
#define KC85_MAX_AUDIO_SAMPLES (1024)       /* max number of audio samples in internal sample buffer */
#define KC85_DEFAULT_AUDIO_SAMPLES (128)    /* default number of samples in internal sample buffer */ 
#define KC85_MAX_TAPE_SIZE (64 * 1024)      /* max size of a snapshot file in bytes */
#define KC85_NUM_SLOTS (2)                  /* 2 expansion slots in main unit, each needs one mem_t layer! */
#define KC85_EXP_BUFSIZE (KC85_NUM_SLOTS*64*1024) /* expansion system buffer size (64 KB per slot) */

/* IO bits */
#define KC85_PIO_A_CAOS_ROM        (1<<0)
#define KC85_PIO_A_RAM             (1<<1)
#define KC85_PIO_A_IRM             (1<<2)
#define KC85_PIO_A_RAM_RO          (1<<3)
#define KC85_PIO_A_UNUSED          (1<<4)
#define KC85_PIO_A_TAPE_LED        (1<<5)
#define KC85_PIO_A_TAPE_MOTOR      (1<<6)
#define KC85_PIO_A_BASIC_ROM       (1<<7)
#define KC85_PIO_B_VOLUME_MASK     ((1<<5)-1)
#define KC85_PIO_B_RAM8            (1<<5)  /* KC85/4 only */
#define KC85_PIO_B_RAM8_RO         (1<<6)  /* KC85/4 only */
#define KC85_PIO_B_BLINK_ENABLED   (1<<7)
/* KC85/4 only IO latches */
#define KC85_IO84_SEL_VIEW_IMG     (1<<0)  /* 0: display img0, 1: display img1 */
#define KC85_IO84_SEL_CPU_COLOR    (1<<1)  /* 0: access pixels, 1: access colors */
#define KC85_IO84_SEL_CPU_IMG      (1<<2)  /* 0: access img0, 1: access img1 */
#define KC85_IO84_HICOLOR          (1<<3)  /* 0: hicolor mode off, 1: hicolor mode on */
#define KC85_IO84_SEL_RAM8         (1<<4)  /* select RAM8 block 0 or 1 */
#define KC85_IO84_BLOCKSEL_RAM8    (1<<5)  /* no idea what that does...? */
#define KC85_IO86_RAM4             (1<<0)
#define KC85_IO86_RAM4_RO          (1<<1)
#define KC85_IO86_CAOS_ROM_C       (1<<7)


/* KC85 model types */
typedef enum {
    KC85_TYPE_2,        /* KC85/2 (default) */
    KC85_TYPE_3,        /* KC85/3 */
    KC85_TYPE_4,        /* KC85/4 */
} kc85_type_t;

/* audio sample callback */
typedef void (*kc85_audio_callback_t)(const float* samples, int num_samples, void* user_data);
/* callback to apply patches after a snapshot is loaded */
typedef void (*kc85_patch_callback_t)(const char* snapshot_name, void* user_data);

/* config parameters for kc85_init() */
typedef struct {
    kc85_type_t type;           /* default is KC85_TYPE_2 */

    /* video output config (if you don't need display decoding, set pixel_buffer to 0) */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    /* optional user-data for callback functions */
    void* user_data;

    /* audio output config (if you don't want audio, set audio_cb to zero) */
    kc85_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is KC85_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* audio volume (0.0 .. 1.0), default is 0.4 */
    
    /* an optional callback to be invoked after a snapshot file is loaded to apply patches */
    kc85_patch_callback_t patch_cb;

    /* ROM images */
    const void* rom_caos22;             /* CAOS 2.2 (used in KC85/2) */
    const void* rom_caos31;             /* CAOS 3.1 (used in KC85/3) */
    const void* rom_caos42c;            /* CAOS 4.2 at 0xC000 (KC85/4) */
    const void* rom_caos42e;            /* CAOS 4.2 at 0xE000 (KC85/4) */
    const void* rom_kcbasic;            /* same BASIC version for KC85/3 and KC85/4 */
    int rom_caos22_size;
    int rom_caos31_size;
    int rom_caos42c_size;
    int rom_caos42e_size;
    int rom_kcbasic_size;
} kc85_desc_t;

/* KC85 expansion module types */
typedef enum {
    KC85_MODULE_NONE,
    KC85_MODULE_M006_BASIC,         /* BASIC+CAOS 16K ROM module for the KC85/2 (id=0xFC) */
    KC85_MODULE_M011_64KBYE,        /* 64 KByte RAM expansion (id=0xF6) */
    KC85_MODULE_M012_TEXOR,         /* TEXOR text editing (id=0xFB) */
    KC85_MODULE_M022_16KBYTE,       /* 16 KByte RAM expansion (id=0xF4) */
    KC85_MODULE_M026_FORTH,         /* FORTH IDE (id=0xFB) */
    KC85_MODULE_M027_DEVELOPMENT,   /* Assembler IDE (id=0xFB) */

    KC85_MODULE_NUM,
} kc85_module_type_t;

/* KC85 expansion module attributes */
typedef struct {
    kc85_module_type_t type;
    uint8_t id;                     /* id of currently inserted module */
    bool writable;                  /* RAM or ROM module */
    uint8_t addr_mask;              /* the module's address mask */
    int size;                       /* the module's byte size */
} kc85_module_t;

/* KC85 expansion slot */
typedef struct {
    uint8_t addr;                   /* 0x0C (left slot) or 0x08 (right slot) */
    uint8_t ctrl;                   /* current control byte */
    uint32_t buf_offset;            /* byte-offset in expansion system data buffer */
    kc85_module_t mod;              /* attributes of currently inserted module */
} kc85_slot_t;

/* KC85 expansion system state:
    NOTE that each expansion slot needs its own memory-mapping layer starting 
    at layer 1 (layer 0 is used by the base system)
*/
typedef struct {
    kc85_slot_t slot[KC85_NUM_SLOTS];   /* slots 0x08 and 0x0C in KC85 main unit */
    uint32_t buf_top;                   /* offset of free area in expansion buffer (kc85_t.exp_buf[]) */
} kc85_exp_t;

/* KC85 emulator state */
typedef struct {
    z80_t cpu;
    z80ctc_t ctc;
    z80pio_t pio;
    beeper_t beeper_1;
    beeper_t beeper_2;

    bool valid;
    kc85_type_t type;
    uint8_t pio_a;          /* current PIO-A value, used for bankswitching */
    uint8_t pio_b;          /* current PIO-B value, used for bankswitching */
    uint8_t io84;           /* byte latch at port 0x84, only on KC85/4 */
    uint8_t io86;           /* byte latch at port 0x86, only on KC85/4 */
    bool blink_flag;        /* foreground color blinking flag toggled by CTC */

    int scanline_period;
    int scanline_counter;
    int cur_scanline;

    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    kc85_exp_t exp;         /* expansion module system */

    uint32_t* pixel_buffer;
    void* user_data;
    kc85_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[KC85_MAX_AUDIO_SAMPLES];
    kc85_patch_callback_t patch_cb;

    uint8_t ram[8][0x4000];             /* up to 8 16-KByte RAM banks */
    uint8_t rom_basic[0x2000];          /* 8 KByte BASIC ROM (KC85/3 and /4 only) */
    uint8_t rom_caos_c[0x1000];         /* 4 KByte CAOS ROM at 0xC000 (KC85/4 only) */
    uint8_t rom_caos_e[0x2000];         /* 8 KByte CAOS ROM at 0xE000 */
    uint8_t exp_buf[KC85_EXP_BUFSIZE];  /* expansion system RAM/ROM */
} kc85_t;

/* initialize a new KC85 instance */
void kc85_init(kc85_t* sys, const kc85_desc_t* desc);
/* discard a KC85 instance */
void kc85_discard(kc85_t* sys);
/* reset a KC85 instance */
void kc85_reset(kc85_t* sys);
/* run KC85 emulation for a given number of microseconds */
void kc85_exec(kc85_t* sys, uint32_t micro_seconds);
/* send a key-down event */
void kc85_key_down(kc85_t* sys, int key_code);
/* send a key-up event */
void kc85_key_up(kc85_t* sys, int key_code);
/* insert a RAM module (slot must be 0x08 or 0x0C) */
bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type);
/* insert a ROM module (slot must be 0x08 or 0x0C) */
bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type, const void* rom_ptr, int rom_size);
/* remove module in slot */
bool kc85_remove_module(kc85_t* sys, uint8_t slot);
/* lookup slot struct by slot address (0x08 or 0x0C) */
kc85_slot_t* kc85_slot_by_addr(kc85_t* sys, uint8_t slot_addr);
/* return true if a slot contains a module */
bool kc85_slot_occupied(kc85_t* sys, uint8_t slot_addr);
/* test if module in slot is currently mapped to CPU-visible memory */
bool kc85_slot_cpu_visible(kc85_t* sys, uint8_t slot_addr);
/* compute the current CPU address of module in slot (0 if no active module in slot) */
uint16_t kc85_slot_cpu_addr(kc85_t* sys, uint8_t slot_addr);
/* load a .KCC or .TAP snapshot file into the emulator */
bool kc85_quickload(kc85_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
