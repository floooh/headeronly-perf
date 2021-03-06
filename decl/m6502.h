#pragma once
/*#
    # m6502.h

    MOS Technology 6502 / 6510 CPU emulator.

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

    ## Emulated Pins

    ***********************************
    *           +-----------+         *
    *   IRQ --->|           |---> A0  *
    *   NMI --->|           |...      *
    *    RDY--->|           |---> A15 *
    *    RW <---|           |         *
    *  SYNC <---|           |         *
    *           |           |<--> D0  *
    *   (P0)<-->|           |...      *
    *        ...|           |<--> D7  *
    *   (P5)<-->|           |         *
    *           +-----------+         *
    ***********************************

    The input/output P0..P5 pins only exist on the m6510.

    If the RDY pin is active (1) the CPU will loop on the next read
    access until the pin goes inactive.

    ## Notes

    Stored here for later referencer:

    - https://www.pagetable.com/?p=39
    
    Maybe it makes sense to rewrite the code-generation python script with
    a real 'decode' ROM?

    ## Functions
    ~~~C
    void m6502_init(m6502_t* cpu, m6502_desc_t* desc)
    ~~~
        Initialize a m6502_t instance, the desc structure provides initialization
        attributes:
            ~~~C
            typedef struct {
                m6502_tick_t tick_cb;       // the CPU tick callback
                bool bcd_disabled;          // set to true if BCD mode is disabled
                m6510_in_t in_cb;           // optional port IO input callback (only on m6510)
                m6510_out_t out_cb;         // optional port IO output callback (only on m6510)
                uint8_t m6510_io_pullup;    // IO port bits that are 1 when reading
                uint8_t m6510_io_floating;  // unconnected IO port pins
                void* user_data;            // optional user-data for callbacks
            } m6502_desc_t;
            ~~~

        To emulate a m6510 you must provide port IO callbacks in _in_cb_ and _out_cb_,
        and should initialize the m6510_io_pullup and m6510_io_floating members.

    ~~~C
    void m6502_reset(m6502_t* cpu)
    ~~~
        Reset the m6502 instance.

    ~~~C
    uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks)
    ~~~
        Execute instructions until the requested number of _ticks_ is reached,
        or a trap has been hit. Return number of executed cycles. To check if a trap
        has been hit, test the m6502_t.trap_id member on >= 0. 
        During execution the tick callback will be called for each clock cycle
        with the current CPU pin bitmask. The tick callback function must inspect
        the pin bitmask, perform memory requests and if necessary update the
        data bus pins. Finally the tick callback returns the (optionally
        modified) pin bitmask. If the tick callback sets the RDY pin (1),
        and the current tick is a read-access, the CPU will loop until the
        RDY pin is (0). The RDY pin is automatically cleared before the
        tick function is called for a read-access.

    ~~~C
    uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins)
    ~~~
        For the m6510, call this function from inside the tick callback when the
        CPU wants to access the special memory location 0 and 1 (these are mapped
        to the IO port control registers of the m6510). m6510_iorq() may call the
        input/output callback functions provided in m6510_init().

    ~~~C
    void m6502_set_trap(m6502_t* cpu, int trap_id, uint16_t addr)
    ~~~
        Set a trap breakpoint at a 16-bit CPU address. Up to 8 trap breakpoints
        can be set. After each instruction, the current PC will be checked
        against all valid trap points, and if there is a match, m6502_exec() will
        return early, and the trap_id member of m6502_t will be >= 0. This can be
        used to set debugger breakpoints, or call out into native host system
        code for other reasons (for instance replacing operating system functions
        like loading game files).

    ~~~C
    bool m6502_has_trap(m6502_t* cpu, int trap_id)
    ~~~
        Return true if a trap with number _trap_id_ is currently set.

    ~~~C
    void m6502_clear_trap(m6502_t* cpu, int trap_id)
    ~~~
        Clear the trap with number _trap_id_.

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

/* address lines */
#define M6502_A0  (1ULL<<0)
#define M6502_A1  (1ULL<<1)
#define M6502_A2  (1ULL<<2)
#define M6502_A3  (1ULL<<3)
#define M6502_A4  (1ULL<<4)
#define M6502_A5  (1ULL<<5)
#define M6502_A6  (1ULL<<6)
#define M6502_A7  (1ULL<<7)
#define M6502_A8  (1ULL<<8)
#define M6502_A9  (1ULL<<9)
#define M6502_A10 (1ULL<<10)
#define M6502_A11 (1ULL<<11)
#define M6502_A12 (1ULL<<12)
#define M6502_A13 (1ULL<<13)
#define M6502_A14 (1ULL<<14)
#define M6502_A15 (1ULL<<15)

/*--- data lines ------*/
#define M6502_D0  (1ULL<<16)
#define M6502_D1  (1ULL<<17)
#define M6502_D2  (1ULL<<18)
#define M6502_D3  (1ULL<<19)
#define M6502_D4  (1ULL<<20)
#define M6502_D5  (1ULL<<21)
#define M6502_D6  (1ULL<<22)
#define M6502_D7  (1ULL<<23)

/*--- control pins ---*/
#define M6502_RW    (1ULL<<24)
#define M6502_SYNC  (1ULL<<25)
#define M6502_IRQ   (1ULL<<26)
#define M6502_NMI   (1ULL<<27)
#define M6502_RDY   (1ULL<<28)
#define M6510_AEC   (1ULL<<29)

/* bit mask for all CPU pins */
#define M6502_PIN_MASK (0xFFFFFFFF)

/*--- status indicator flags ---*/
#define M6502_CF (1<<0)   /* carry */
#define M6502_ZF (1<<1)   /* zero */
#define M6502_IF (1<<2)   /* IRQ disable */
#define M6502_DF (1<<3)   /* decimal mode */
#define M6502_BF (1<<4)   /* BRK command */
#define M6502_XF (1<<5)   /* unused */
#define M6502_VF (1<<6)   /* overflow */
#define M6502_NF (1<<7)   /* negative */

/* max number of trap points */
#define M6502_MAX_NUM_TRAPS (8)

/* tick callback function typedef */
typedef uint64_t (*m6502_tick_t)(uint64_t pins, void* user_data);
/* callbacks for M6510 port I/O */
typedef void (*m6510_out_t)(uint8_t data, void* user_data);
typedef uint8_t (*m6510_in_t)(void* user_data);

/* the desc structure provided to m6502_init() */
typedef struct {
    m6502_tick_t tick_cb;   /* the CPU tick callback */
    bool bcd_disabled;      /* set to true if BCD mode is disabled */
    m6510_in_t in_cb;       /* optional port IO input callback (only on m6510) */
    m6510_out_t out_cb;     /* optional port IO output callback (only on m6510) */
    uint8_t m6510_io_pullup;    /* IO port bits that are 1 when reading */
    uint8_t m6510_io_floating;  /* unconnected IO port pins */
    void* user_data;        /* optional user-data for callbacks */
} m6502_desc_t;

/* mutable tick state */
typedef struct {
    uint64_t PINS;
    uint8_t A,X,Y,S,P;      /* 8-bit registers */
    uint16_t PC;            /* 16-bit program counter */
    /* state of interrupt enable flag at the time when the IRQ pin is sampled,
       this is used to implement 'delayed IRQ response'
       (see: https://wiki.nesdev.com/w/index.php/CPU_interrupts)
    */
    uint8_t pi;
    bool bcd_enabled;       /* this is actually not mutable but needed when ticking */
} m6502_state_t;

/* M6502 CPU state */
typedef struct {
    m6502_state_t state;
    m6502_tick_t tick;
    void* user_data;

    /* the m6510 IO port stuff */
    m6510_in_t in_cb;
    m6510_out_t out_cb;
    uint8_t io_ddr;     /* 1: output, 0: input */
    uint8_t io_port;
    uint8_t io_pullup;
    uint8_t io_floating;
    uint8_t io_drive;

    /* trap points */
    bool trap_valid[M6502_MAX_NUM_TRAPS];
    uint16_t trap_addr[M6502_MAX_NUM_TRAPS];
    int trap_id;        /* index of trap hit (-1 if no trap) */
} m6502_t;

/* initialize a new m6502 instance */
extern void m6502_init(m6502_t* cpu, m6502_desc_t* desc);
/* reset an existing m6502 instance */
extern void m6502_reset(m6502_t* cpu);
/* set a trap point */
extern void m6502_set_trap(m6502_t* cpu, int trap_id, uint16_t addr);
/* clear a trap point */
extern void m6502_clear_trap(m6502_t* cpu, int trap_id);
/* return true if a trap is valid */
extern bool m6502_has_trap(m6502_t* cpu, int trap_id);
/* execute instruction for at least 'ticks' or trap hit, return number of executed ticks */
extern uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks);
/* perform m6510 port IO (only call this if M6510_CHECK_IO(pins) is true) */
extern uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins);

/* extract 16-bit address bus from 64-bit pins */
#define M6502_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define M6502_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define M6502_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6502_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* return a pin mask with control-pins, address and data bus */
#define M6502_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))

/* M6510: check for IO port access to address 0 or 1 */
#define M6510_CHECK_IO(p) ((p&0xFFFEULL)==0)

#ifdef __cplusplus
} /* extern "C" */
#endif
