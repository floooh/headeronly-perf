#pragma once
/*#
    # m6526.h

    MOS Technology 6526 Complex Interface Adapter (CIA)

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

    ************************************
    *           +-----------+          *
    *    CS --->|           |<--- FLAG *
    *    RW --->|           |---> PC   *
    *   RES --->|           |---> SP   *
    *   IRQ <---|           |<--- TOD  *
    *           |           |<--- CNT  *
    *           |           |          *
    *   RS0 --->|   M6526   |<--> PA0  *
    *   RS1 --->|           |...       *
    *   RS2 --->|           |<--> PA7  *
    *   RS3 --->|           |          *
    *           |           |<--> PB0  *
    *   DB0 --->|           |...       *
    *        ...|           |<--> PB7  *
    *   DB7 --->|           |          *
    *           +-----------+          *
    ************************************

    ## NOT IMPLEMENTED:

    - PC pin
    - time of day clock
    - serial port
    - no external counter trigger via CNT pin

    ## LINKS:
    - https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html
    - https://ist.uwaterloo.ca/~schepers/MJK/cia6526.html

    TODO: Documentation
    
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

/* register select same as lower 4 shared address bus bits */
#define M6526_RS0   (1ULL<<0)
#define M6526_RS1   (1ULL<<1)
#define M6526_RS2   (1ULL<<2)
#define M6526_RS3   (1ULL<<3)
#define M6526_RS    (M6526_RS3|M6526_RS2|M6526_RS1|M6526_RS0)

/* data bus pins shared with CPU */
#define M6526_D0    (1ULL<<16)
#define M6526_D1    (1ULL<<17)
#define M6526_D2    (1ULL<<18)
#define M6526_D3    (1ULL<<19)
#define M6526_D4    (1ULL<<20)
#define M6526_D5    (1ULL<<21)
#define M6526_D6    (1ULL<<22)
#define M6526_D7    (1ULL<<23)

/* control pins shared with CPU */
#define M6526_RW    (1ULL<<24)      /* same as M6502_RW */
#define M6526_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

/* chip-specific control pins */
#define M6526_CS    (1ULL<<40)
#define M6526_FLAG  (1ULL<<41)
#define M6526_PC    (1ULL<<42)
#define M6526_SP    (1ULL<<43)
#define M6526_TOD   (1ULL<<44)
#define M6526_CNT   (1ULL<<45)

/* register indices */
#define M6526_REG_PRA       (0)     /* peripheral data reg A */
#define M6526_REG_PRB       (1)     /* peripheral data reg B */
#define M6526_REG_DDRA      (2)     /* data direction reg A */
#define M6526_REG_DDRB      (3)     /* data direction reg B */
#define M6526_REG_TALO      (4)     /* timer A low register */
#define M6526_REG_TAHI      (5)     /* timer A high register */
#define M6526_REG_TBLO      (6)     /* timer B low register */
#define M6526_REG_TBHI      (7)     /* timer B high register */
#define M6526_REG_TOD10TH   (8)     /* 10ths of seconds register */
#define M6526_REG_TODSEC    (9)     /* seconds register */
#define M6526_REG_TODMIN    (10)    /* minutes register */
#define M6526_REG_TODHR     (11)    /* hours am/pm register */
#define M6526_REG_SDR       (12)    /* serial data register */
#define M6526_REG_ICR       (13)    /* interrupt control register */
#define M6526_REG_CRA       (14)    /* control register A */
#define M6526_REG_CRB       (15)    /* control register B */

/* port in/out callbacks */
#define M6526_PORT_A (0)
#define M6526_PORT_B (1)
typedef uint8_t (*m6526_in_t)(int port_id, void* user_data);
typedef void (*m6526_out_t)(int port_id, uint8_t data, void* user_data);

/* m6526 initialization parameters */
typedef struct {
    m6526_in_t in_cb;
    m6526_out_t out_cb;
    void* user_data;
} m6526_desc_t;

/* I/O port state */
typedef struct {
    uint8_t reg;        /* port register */
    uint8_t ddr;        /* data direction register */
    uint8_t inp;        /* input latch */
    uint8_t last_out;   /* last value to reduce call to out callback */
} m6526_port_t;

/* timer state */
typedef struct {
    uint16_t latch;     /* 16-bit initial value latch */
    uint16_t counter;   /* 16-bit counter */
    uint8_t cr;         /* control register */
    bool t_bit;         /* toggles between true and false when counter underflows */
    bool t_out;         /* true for 1 cycle when counter underflow */
    uint8_t pip_count;      /* 2-cycle delay pipeline, output is 'counter active' */
    uint8_t pip_oneshot;    /* 1-cycle delay pipeline, output is 'oneshot mode active' */
    uint8_t pip_load;       /* 1-cycle delay pipeline, output is 'force load' */
} m6526_timer_t;

/* interrupt state */
typedef struct {
    uint8_t imr;        /* interrupt mask */
    uint8_t imr1;       /* one cycle delay for imr updates */
    uint8_t icr;        /* interrupt control register */
    uint8_t pip_irq;    /* 1-cycle delay pipeline to request irq */
    bool flag;          /* last state of flag bit, to detect edge */
} m6526_int_t;

/* m6526 state */
typedef struct {
    m6526_port_t pa;
    m6526_port_t pb;
    m6526_timer_t ta;
    m6526_timer_t tb;
    m6526_int_t intr;
    m6526_in_t in_cb;
    m6526_out_t out_cb;
    void* user_data;
} m6526_t;

/* extract 8-bit data bus from 64-bit pins */
#define M6526_GET_DATA(p) ((uint8_t)(p>>16))
/* merge 8-bit data bus value into 64-bit pins */
#define M6526_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
/* merge 4-bit register-select address into 64-bit pins */
#define M6526_SET_ADDR(p,d) {p=((p&~0xF)|(d&0xF));}

/* initialize a new m6526_t instance */
extern void m6526_init(m6526_t* c, m6526_desc_t* desc);
/* reset an existing m6526_t instance */
extern void m6526_reset(m6526_t* c);
/* perform an IO request */
extern uint64_t m6526_iorq(m6526_t* c, uint64_t pins);
/* tick the m6526_t instance, return true if interrupt requested */
extern uint64_t m6526_tick(m6526_t* c, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
