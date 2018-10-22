#pragma once
/*#
    # z80.h

    Header-only Z80 CPU emulator written in C.

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

    ## Emulated Pins
    ***********************************
    *           +-----------+         *
    * M1    <---|           |---> A0  *
    * MREQ  <---|           |---> A1  *
    * IORQ  <---|           |---> A2  *
    * RD    <---|           |---> ..  *
    * WR    <---|    Z80    |---> A15 *
    * HALT  <---|           |         *
    * WAIT  --->|           |<--> D0  *
    * INT   --->|           |<--> D1  *
    *           |           |<--> ... *
    *           |           |<--> D7  *
    *           +-----------+         *
    ***********************************

    ## Not Emulated
    - refresh cycles (RFSH pin)
    - interrupt mode 0
    - bus request/acknowledge (BUSRQ and BUSAK pins)
    - the RESET pin is currently not tested, call the z80_reset() 
      function instead

    ## Functions
    ~~~C
    void z80_init(z80_t* cpu, z80_desc_t* desc)
    ~~~
        Initializes a new Z80 CPU instance. The z80_desc_t struct
        provides initialization attributes:
            ~~~C
            typedef struct {
                z80_tick_t tick_cb; // the CPU tick callback
                void* user_data;    // user data arg handed to callbacks
            } z80_desc_t;
            ~~~
        The tick_cb function will be called from inside z80_exec().

    ~~~C
    void z80_reset(z80_t* cpu)
    ~~~
        Resets the Z80 CPU instance. 

    ~~~C
    uint32_t z80_exec(z80_t* cpu, uint32_t num_ticks)
    ~~~
        Starts executing instructions until num_ticks is reached or the PC
        hits a trap, returns the number of executed ticks. The number of
        executed ticks will be greater or equal to num_ticks (unless a trap
        has been hit), because complete instructions will be executed. During
        execution the tick callback will be invoked one or multiple times
        (usually once per machine cycle, but also for 'filler ticks' or wait
        state ticks). To check if a trap has been hit, test whether the 
        z80_t.trap_id is >= 0.
        NOTE: the z80_exec() function may return in the 'middle' of an
        DD/FD extended instruction (right after the prefix byte). If this
        is the case, z80_opdone() will return false.

    ~~~C
    bool z80_opdone(z80_t* cpu)
    ~~~
        Return true if z80_exec() has returned at the end of an instruction,
        and false if the CPU is in the middle of a DD/FD prefix instruction.

    ~~~C
    void z80_set_x(z80_t* cpu, uint8_t val)
    void z80_set_xx(z80_t* cpu, uint16_t val)
    uint8_t z80_x(z80_t* cpu)
    uint16_t z80_xx(z80_t* cpu)
    ~~~
        Set and get Z80 registers and flags.

    ~~~C
    void z80_set_trap(z80_t* cpu, int trap_id, uint16_t addr)
    ~~~
        Set a trap breakpoint at a 16-bit CPU address. Up to 4 trap breakpoints
        can be set. After each instruction, the current PC will be checked
        against all valid trap points, and if there is a match, z80_exec() will
        return early, and the trap_id member of z80_t will be >= 0. This can be
        used to set debugger breakpoints, or call out into native host system
        code for other reasons (for instance replacing operating system functions
        like loading game files). 

    ~~~C
    void z80_clear_trap(z80_t* cpu, int trap_id)
    ~~~
        Clear a trap point.

    ~~~C
    bool z80_has_trap(z80_t* cpu, int trap_id)
    ~~~
        Return true if a trap has been set for the given trap_id.

    ## Macros
    ~~~C
    Z80_SET_ADDR(pins, addr)
    ~~~
        set 16-bit address bus pins in 64-bit pin mask

    ~~~C
    Z80_GET_ADDR(pins)
    ~~~
        extract 16-bit address bus value from 64-bit pin mask

    ~~~C
    Z80_SET_DATA(pins, data)
    ~~~
        set 8-bit data bus pins in 64-bit pin mask

    ~~~C
    Z80_GET_DATA(pins)
    ~~~
        extract 8-bit data bus value from 64-bit pin mask

    ~~~C
    Z80_MAKE_PINS(ctrl, addr, data)
    ~~~
        build 64-bit pin mask from control-, address- and data-pins

    ~~~C
    Z80_DAISYCHAIN_BEGIN(pins)
    ~~~
        used in tick function at start of interrupt daisy-chain block

    ~~~C
    Z80_DAISYCHAIN_END(pins)
    ~~~
        used in tick function at end of interrupt daisy-chain block

    ## The Tick Callback 

    The tick function is called for one or multiple time cycles
    and connects the Z80 to the outside world. Usually one call
    of the tick function corresponds to one machine cycle, 
    but this is not always the case. The tick functions takes
    2 arguments:

    - num_ticks: the number of time cycles for this callback invocation
    - pins: a 64-bit integer with CPU pins (address- and data-bus pins,
        and control pins)

    A simplest-possible tick callback which just performs memory read/write
    operations on a 64kByte byte array looks like this:

    ~~~C
    uint8_t mem[1<<16] = { 0 };
    uint64_t tick(int num_ticks, uint64_t pins, void* user_data) {
        if (pins & Z80_MREQ) {
            if (pins & Z80_RD) {
                Z80_SET_DATA(pins, mem[Z80_GET_ADDR(pins)]);
            }
            else if (pins & Z80_WR) {
                mem[Z80_GET_ADDR(pins)] = Z80_GET_DATA(pins);
            }
        }
        else if (pins & Z80_IORQ) {
            // FIXME: perform device I/O
        }
        return pins;
    }
    ~~~

    The tick callback inspects the pins, performs the requested actions
    (memory read/write and input/output), modifies the pin bitmask
    with requests for the CPU (inject wait states, or request an
    interrupt), and finally returns the pin bitmask back to the 
    CPU emulation.

    The following pin bitmasks are relevant for the tick callback:

    - **MREQ|RD**: This is a memory read cycle, the tick callback must 
      put the byte at the memory address indicated by the address
      bus pins A0..A15 (bits 0..15) into the data bus 
      pins (D0..D7). If the M1 pin is also set, then this
      is an opcode fetch machine cycle (4 clock ticks), otherwise
      this is a normal memory read machine cycle (3 clock ticks)
    - **MREQ|WR**: This is a memory write machine cycle, the tick
      callback must write the byte in the data bus pins (D0..D7)
      to the memory location in the address bus pins (A0..A15). 
      A memory write machine cycle is 3 clock-ticks.
    - **IORQ|RD**: This is a device-input machine cycle, the 16-bit
      port number is in the address bus pins (A0..A15), and the
      tick callback must write the input-byte to the data bus
      pins (D0..D7). An input machine cycle is 4 clock-ticks.
    - **IORQ|WR**: This is a device-output machine cycle, the data
      bus pins (D0..D7) contains the byte to be output
      at the port in the address-bus pins (A0..A15). An output
      machine cycle is 4 cycles.

    Interrupt handling requires to inspect and set additional
    pins, more on that below.

    To inject wait states, execute the additional cycles in the
    CPU tick callback, and set the number of wait states
    with the Z80_SET_WAIT() macro on the returned CPU pins.
    Up to 7 wait states can be injected per machine cycle.

    Note that not all calls to the tick callback have one
    of the above pin bit patterns set. The CPU may need
    to execute filler- or processing ticks which are
    not memory-, IO- or interrupt-handling operations.

    This may happen in the following situations:
    - opcode fetch machine cycles are always a single callback
      invocation of 4 cycles with the M1|MREQ|RD pins set, however
      in a real Z80, some opcode fetch machine cycles are 5 or 6
      cycles long, in this case, the tick callback will be called
      again without control pins set and a tick count of 1 or 2
    - some instructions require additional processing ticks which
      are not memory- or IO-operations, in this case the tick
      callback may be called for with any number of ticks, but
      without activated control pins

    ## Interrupt Handling

    The interrupt 'daisy chain protocol' is entirely implemented
    in the tick callback (usually the actual interrupt daisy chain
    handling happens in the Z80 chip-family emulators, but the
    tick callback needs to invoke their interrupt handling functions).

    An interrupt request/acknowledge cycle for (most common)
    interrupt mode 2 looks like this:

    - an interrupt is requested from inside the tick callback by
      setting the INT pin in any tick callback invocation (the 
      INT pins remains active until the end of the current instruction)
    - the CPU emulator checks the INT pin at the end of the current
      instruction, if the INT pin is active, an interrupt-request/acknowledge
      machine cycle is executed which results in additional tick
      callback invocations to perform the interrupt-acknowledge protocol
    - the interrupt-controller device with pending interrupt scans the
      pin bits for M1|IORQ during the tick callback, and if active,
      places the interrupt vector low-byte on the data bus pins
    - back in the CPU emulation, the interrupt request is completed by
      constructing the complete 16-bit interrupt vector, reading the
      address of the interrupt service routine from there, and setting
      the PC register to that address (the next executed instruction
      is then the first instruction of the interrupt service routine)

    There are 2 virtual pins for the interrupt daisy chain protocol:

    - **IEIO** (Interrupt Enable In/Out): This combines the IEI and IEO 
      pins found on Z80 chip-family members and is used to disable
      interrupts for lower-priority interrupt controllers in the
      daisy chain if a higher priority device is currently negotiating
      interrupt handling with the CPU. The IEIO pin always starts
      in active state at the start of the daisy chain, and is handed
      from one interrupt controller to the next in order of 
      daisy-chain priority. The first interrupt controller with
      active interrupt handling clears the IEIO bit, which prevent
      the 'downstream' lower-priority interrupt controllers from
      issuing interrupt requests.
    - **RETI** (Return From Interrupt): The virtual RETI pin is set
      by the CPU when it decodes a RETI instruction. This is scanned
      by the interrupt controller which is currently 'under service'
      to enable interrupt handling for lower-priority devices
      in the daisy chain. In a real Z80 system this the interrupt
      controller chips perform their own simple instruction decoding
      to detect RETI instructions.

    The CPU tick callback is the heart of emulation, for complete
    tick callback examples check the emulators and tests here:
    
    http://github.com/floooh/chips-test

    http://github.com/floooh/yakc

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

/*--- callback function typedefs ---*/
typedef uint64_t (*z80_tick_t)(int num_ticks, uint64_t pins, void* user_data);

/*--- address lines ---*/
#define Z80_A0  (1ULL<<0)
#define Z80_A1  (1ULL<<1)
#define Z80_A2  (1ULL<<2)
#define Z80_A3  (1ULL<<3)
#define Z80_A4  (1ULL<<4)
#define Z80_A5  (1ULL<<5)
#define Z80_A6  (1ULL<<6)
#define Z80_A7  (1ULL<<7)
#define Z80_A8  (1ULL<<8)
#define Z80_A9  (1ULL<<9)
#define Z80_A10 (1ULL<<10)
#define Z80_A11 (1ULL<<11)
#define Z80_A12 (1ULL<<12)
#define Z80_A13 (1ULL<<13)
#define Z80_A14 (1ULL<<14)
#define Z80_A15 (1ULL<<15)

/*--- data lines ------*/
#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

/*--- control pins ---*/

/* system control pins */
#define  Z80_M1    (1ULL<<24)       /* machine cycle 1 */
#define  Z80_MREQ  (1ULL<<25)       /* memory request */
#define  Z80_IORQ  (1ULL<<26)       /* input/output request */
#define  Z80_RD    (1ULL<<27)       /* read */
#define  Z80_WR    (1ULL<<28)       /* write */
#define  Z80_CTRL_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR)

/* CPU control pins */
#define  Z80_HALT  (1ULL<<29)       /* halt state */
#define  Z80_INT   (1ULL<<30)       /* interrupt request */
#define  Z80_NMI   (1ULL<<31)       /* non-maskable interrupt */
#define  Z80_BUSREQ (1ULL<<32)      /* bus request */
#define  Z80_BUSACK (1ULL<<33)      /* bus acknowledge */

/* up to 7 wait states can be injected per machine cycle */
#define Z80_WAIT0   (1ULL<<34)
#define Z80_WAIT1   (1ULL<<35)
#define Z80_WAIT2   (1ULL<<36)
#define Z80_WAIT_SHIFT (34)
#define Z80_WAIT_MASK (Z80_WAIT0|Z80_WAIT1|Z80_WAIT2)

/* interrupt-related 'virtual pins', these don't exist on the Z80 */
#define Z80_IEIO    (1ULL<<37)      /* unified daisy chain 'Interrupt Enable In+Out' */
#define Z80_RETI    (1ULL<<38)      /* cpu has decoded a RETI instruction */

/* bit mask for all CPU bus pins */
#define Z80_PIN_MASK ((1ULL<<40)-1)

/*--- status indicator flags ---*/
#define Z80_CF (1<<0)           /* carry */
#define Z80_NF (1<<1)           /* add/subtract */
#define Z80_VF (1<<2)           /* parity/overflow */
#define Z80_PF Z80_VF
#define Z80_XF (1<<3)           /* undocumented bit 3 */
#define Z80_HF (1<<4)           /* half carry */
#define Z80_YF (1<<5)           /* undocumented bit 5 */
#define Z80_ZF (1<<6)           /* zero */
#define Z80_SF (1<<7)           /* sign */

#define Z80_MAX_NUM_TRAPS (4)

/* initialization attributes */
typedef struct {
    z80_tick_t tick_cb;
    void* user_data;
} z80_desc_t;

/* Z80 CPU state */
typedef struct {
    /* tick callback */
    z80_tick_t tick;
    /* main register bank (BC, DE, HL, FA) */
    uint64_t bc_de_hl_fa;   /* B:63..56 C:55..48 D:47..40 E:39..32 H:31..24 L:23..16: F:15..8, A:7..0 */
    /* shadow register bank (BC', DE', HL', FA') */
    uint64_t bc_de_hl_fa_;
    /* IR,WZ,SP,PC */
    uint64_t wz_ix_iy_sp;
    /* control bits,IM,IY,IX */
    uint64_t im_ir_pc_bits;
    /* last pin state (only for debug inspection) */
    uint64_t pins;
    void* user_data;
    int trap_id;
    uint64_t trap_addr;
} z80_t;

/* initialize a new z80 instance */
extern void z80_init(z80_t* cpu, z80_desc_t* desc);
/* reset an existing z80 instance */
extern void z80_reset(z80_t* cpu);
/* set a trap point */
extern void z80_set_trap(z80_t* cpu, int trap_id, uint16_t addr);
/* clear a trap point */
extern void z80_clear_trap(z80_t* cpu, int trap_id);
/* return true if a trap is valid */
extern bool z80_has_trap(z80_t* cpu, int trap_id);
/* execute instructions for at least 'ticks', but at least one, return executed ticks */
extern uint32_t z80_exec(z80_t* cpu, uint32_t ticks);
/* return false if z80_exec() returned in the middle of an extended intruction */
extern bool z80_opdone(z80_t* cpu);

/* register access functions */
extern void z80_set_a(z80_t* cpu, uint8_t v);
extern void z80_set_f(z80_t* cpu, uint8_t v);
extern void z80_set_l(z80_t* cpu, uint8_t v);
extern void z80_set_h(z80_t* cpu, uint8_t v);
extern void z80_set_e(z80_t* cpu, uint8_t v);
extern void z80_set_d(z80_t* cpu, uint8_t v);
extern void z80_set_c(z80_t* cpu, uint8_t v);
extern void z80_set_b(z80_t* cpu, uint8_t v);
extern void z80_set_fa(z80_t* cpu, uint16_t v);
extern void z80_set_af(z80_t* cpi, uint16_t v);
extern void z80_set_hl(z80_t* cpu, uint16_t v);
extern void z80_set_de(z80_t* cpu, uint16_t v);
extern void z80_set_bc(z80_t* cpu, uint16_t v);
extern void z80_set_fa_(z80_t* cpu, uint16_t v);
extern void z80_set_af_(z80_t* cpi, uint16_t v);
extern void z80_set_hl_(z80_t* cpu, uint16_t v);
extern void z80_set_de_(z80_t* cpu, uint16_t v);
extern void z80_set_bc_(z80_t* cpu, uint16_t v);
extern void z80_set_pc(z80_t* cpu, uint16_t v);
extern void z80_set_wz(z80_t* cpu, uint16_t v);
extern void z80_set_sp(z80_t* cpu, uint16_t v);
extern void z80_set_i(z80_t* cpu, uint8_t v);
extern void z80_set_r(z80_t* cpu, uint8_t v);
extern void z80_set_ix(z80_t* cpu, uint16_t v);
extern void z80_set_iy(z80_t* cpu, uint16_t v);
extern void z80_set_im(z80_t* cpu, uint8_t v);
extern void z80_set_iff1(z80_t* cpu, bool b);
extern void z80_set_iff2(z80_t* cpu, bool b);
extern void z80_set_ei_pending(z80_t* cpu, bool b);

extern uint8_t z80_a(z80_t* cpu);
extern uint8_t z80_f(z80_t* cpu);
extern uint8_t z80_l(z80_t* cpu);
extern uint8_t z80_h(z80_t* cpu);
extern uint8_t z80_e(z80_t* cpu);
extern uint8_t z80_d(z80_t* cpu);
extern uint8_t z80_c(z80_t* cpu);
extern uint8_t z80_b(z80_t* cpu);
extern uint16_t z80_fa(z80_t* cpu);
extern uint16_t z80_af(z80_t* cpu);
extern uint16_t z80_hl(z80_t* cpu);
extern uint16_t z80_de(z80_t* cpu);
extern uint16_t z80_bc(z80_t* cpu);
extern uint16_t z80_fa_(z80_t* cpu);
extern uint16_t z80_af_(z80_t* cpu);
extern uint16_t z80_hl_(z80_t* cpu);
extern uint16_t z80_de_(z80_t* cpu);
extern uint16_t z80_bc_(z80_t* cpu);
extern uint16_t z80_pc(z80_t* cpu);
extern uint16_t z80_wz(z80_t* cpu);
extern uint16_t z80_sp(z80_t* cpu);
extern uint8_t z80_i(z80_t* cpu);
extern uint8_t z80_r(z80_t* cpu);
extern uint16_t z80_ix(z80_t* cpu);
extern uint16_t z80_iy(z80_t* cpu);
extern uint8_t z80_im(z80_t* cpu);
extern bool z80_iff1(z80_t* cpu);
extern bool z80_iff2(z80_t* cpu);
extern bool z80_ei_pending(z80_t* cpu);

/* helper macro to start interrupt handling in tick callback */
#define Z80_DAISYCHAIN_BEGIN(pins) if (pins&Z80_M1) { pins|=Z80_IEIO;
/* helper macro to end interrupt handling in tick callback */
#define Z80_DAISYCHAIN_END(pins) pins&=~Z80_RETI; }
/* return a pin mask with control-pins, address and data bus */
#define Z80_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
/* extract 16-bit address bus from 64-bit pins */
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
/* merge 16-bit address bus value into 64-bit pins */
#define Z80_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
/* extract 8-bit data bus from 64-bit pins */
#define Z80_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define Z80_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
/* extract number of wait states from pin mask */
#define Z80_GET_WAIT(p) ((p&Z80_WAIT_MASK)>>Z80_WAIT_SHIFT)
/* set up to 7 wait states in pin mask */
#define Z80_SET_WAIT(p,w) {p=((p&~Z80_WAIT_MASK)|((((uint64_t)w)<<Z80_WAIT_SHIFT)&Z80_WAIT_MASK));}

#ifdef __cplusplus
} /* extern "C" */
#endif
