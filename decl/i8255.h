#pragma once
/*
    i8255.h -- emulates the Intel 8255 PPI chip

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    EMULATED PINS:

                  +-----------+
            CS -->|           |<-> D0
            RD -->|           |...
            WR -->|   i8255   |<-> D7
            A0 -->|           |
            A1 -->|           |
                  |           |
                  +-----------+

    The 24 port A,B and C data pins are not emulated through pin masks, but
    are accessed through callback- and 'active write'-functions (similar to
    z80pio.h).

    NOT IMPLEMENTED:
        - mode 1 (strobed input/output)
        - mode 2 (bi-directional bus)
        - interrupts
    
    FIXME: documentation

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

/*
    Pin definitions.

    Pin locations 0 to 39 are reserved for the CPU. Data bus pins D0..D7
    are shared directly with the CPU.

    Control pin operations (XX: active, --: inactive, ..: don't care)

    | A1 | A0 | RD | WR | CS |
    +----+----+----+----+----+
    | -- | -- | XX | -- | XX |  read: port A -> data bus
    | -- | XX | XX | -- | XX |  read: port B -> data bus
    | XX | -- | XX | -- | XX |  read: port C -> data bus
    | XX | XX | XX | -- | XX |  read: control word -> data bus
    | -- | -- | -- | XX | XX |  write: data bus -> port A
    | -- | XX | -- | XX | XX |  write: data bus -> port B
    | XX | -- | -- | XX | XX |  write: data bus -> port C
    | XX | XX | -- | XX | XX |  write: data bus -> control

    Invoke those operations via to i8255_iorq() function.
*/

/* i8255 control pins */
#define I8255_CS    (1ULL<<40)     /* chip-select, i8255 responds to RW/WR when this is active */
#define I8255_RD    (1ULL<<41)     /* read from PPI */
#define I8255_WR    (1ULL<<42)     /* write to PPI */
#define I8255_A0    (1ULL<<43)     /* address bit 0 */
#define I8255_A1    (1ULL<<44)     /* address bit 1 */

/* data bus pins shared with CPU */
#define I8255_D0  (1ULL<<16)
#define I8255_D1  (1ULL<<17)
#define I8255_D2  (1ULL<<18)
#define I8255_D3  (1ULL<<19)
#define I8255_D4  (1ULL<<20)
#define I8255_D5  (1ULL<<21)
#define I8255_D6  (1ULL<<22)
#define I8255_D7  (1ULL<<23)

/* port names */
#define I8255_PORT_A    (0)
#define I8255_PORT_B    (1)
#define I8255_PORT_C    (2)
#define I8255_NUM_PORTS (3)

/*
    Control word bits

    MODE SELECT (bit 7: 1)

    | C7 | C6 | C5 | C4 | C3 | C2 | C1 | C0 |

    C0..C2: GROUP B control bits:
        C0: port C (lower) in/out:  0=output, 1=input
        C1: port B in/out:          0=output, 1=input
        C2: mode select:            0=mode0 (basic in/out), 1=mode1 (strobed in/out)

    C3..C6: GROUP A control bits:
        C3: port C (upper) in/out:  0=output, 1=input
        C4: port A in/out:          0=output, 1=input
        C5+C6: mode select:         00=mode0 (basic in/out)
                                    01=mode1 (strobed in/out)
                                    1x=mode2 (bi-directional bus)
    
    C7: 1 to for 'mode select'

    INTERRUPT CONTROL (bit 7: 0)

    Interrupt handling is currently not implemented
*/

/* mode select or interrupt control */
#define I8255_CTRL_CONTROL               (1<<7)
#define I8255_CTRL_CONTROL_MODE          (1<<7)
#define I8255_CTRL_CONTROL_BIT           (0)

/* port C (lower) input/output select */
#define I8255_CTRL_CLO              (1<<0)
#define I8255_CTRL_CLO_INPUT        (1<<0)
#define I8255_CTRL_CLO_OUTPUT       (0)

/* port B input/output select */
#define I8255_CTRL_B                (1<<1)
#define I8255_CTRL_B_INPUT          (1<<1)
#define I8255_CTRL_B_OUTPUT         (0)

/* group B mode select */
#define I8255_CTRL_BCLO_MODE        (1<<2)
#define I8255_CTRL_BCLO_MODE_0      (0)
#define I8255_CTRL_BCLO_MIDE_1      (1<<2)

/* port C (upper) input/output select */
#define I8255_CTRL_CHI              (1<<3)
#define I8255_CTRL_CHI_INPUT        (1<<3)
#define I8255_CTRL_CHI_OUTPUT       (0)

/* port A input/output select */
#define I8255_CTRL_A                (1<<4)
#define I8255_CTRL_A_INPUT          (1<<4)
#define I8255_CTRL_A_OUTPUT         (0)

/* group A mode select */
#define I8255_CTRL_ACHI_MODE        ((1<<6)|(1<<5))
#define I8255_CTRL_ACHI_MODE_0      (0)
#define I8255_CTRL_ACHI_MODE_1      (1<<5)
/* otherwise mode 2 */

/* set/reset bit (for I8255_CTRL_CONTROL_BIT) */
#define I8255_CTRL_BIT              (1<<0)
#define I8255_CTRL_BIT_SET          (1<<0)
#define I8255_CTRL_BIT_RESET        (0)

/* callbacks for input/output on ports */
typedef uint8_t (*i8255_in_t)(int port_id, void* user_data);
typedef uint64_t (*i8255_out_t)(int port_id, uint64_t pins, uint8_t data, void* user_data);

/* initialization parameters */
typedef struct {
    i8255_in_t in_cb;       /* port-input callback */
    i8255_out_t out_cb;     /* port-output callback */
    void* user_data;        /* optional user-data for callbacks */
} i8255_desc_t;

/* i8255 state */
typedef struct {
    /* port output latches */
    uint8_t output[I8255_NUM_PORTS];
    /* control word */
    uint8_t control;
    /* last input value (only for debugging) */
    uint8_t dbg_input[I8255_NUM_PORTS];
    /* port-input callback */
    i8255_in_t in_cb;
    /* port-output callback */
    i8255_out_t out_cb;
    /* optional user-data for callbacks */
    void* user_data;
} i8255_t;

/* extract 8-bit data bus from 64-bit pins */
#define I8255_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define I8255_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

/*
    i8255_init

    This initializes a new i8255 instance, which will clear
    the i8255_t struct and put the chip into its reset state.

    ppi     -- pointer to a i8255_t instance
    in_cb   -- function to be called when input on a port is needed
    out_cb  -- function to be called when output on a port is performed
*/
extern void i8255_init(i8255_t* ppi, i8255_desc_t* desc);

/*
    i8255_reset

    Puts the i8255 into the reset state. Clears the control word register
    and puts all ports into input mode.
*/
extern void i8255_reset(i8255_t* ppi);

/*
    i8255_iorq

    Performs a read or write operation on the PPI. The pins 
    CS|RD|WR define whether this is a read or write operation,
    the address pins A0|A1 define the port number, or
    whether the control word is affected. If this is a
    write operation, the data bus pins must contain the value
    to be written.

    Calling this function may cause invocation of the in/out
    callback function, and the data bus pins may be modified
    (if this is a read operation).
*/
extern uint64_t i8255_iorq(i8255_t* ppi, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
