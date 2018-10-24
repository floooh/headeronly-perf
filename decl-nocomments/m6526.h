#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M6526_RS0   (1ULL<<0)
#define M6526_RS1   (1ULL<<1)
#define M6526_RS2   (1ULL<<2)
#define M6526_RS3   (1ULL<<3)
#define M6526_RS    (M6526_RS3|M6526_RS2|M6526_RS1|M6526_RS0)

#define M6526_D0    (1ULL<<16)
#define M6526_D1    (1ULL<<17)
#define M6526_D2    (1ULL<<18)
#define M6526_D3    (1ULL<<19)
#define M6526_D4    (1ULL<<20)
#define M6526_D5    (1ULL<<21)
#define M6526_D6    (1ULL<<22)
#define M6526_D7    (1ULL<<23)

#define M6526_RW    (1ULL<<24)      /* same as M6502_RW */
#define M6526_IRQ   (1ULL<<26)      /* same as M6502_IRQ */

#define M6526_CS    (1ULL<<40)
#define M6526_FLAG  (1ULL<<41)
#define M6526_PC    (1ULL<<42)
#define M6526_SP    (1ULL<<43)
#define M6526_TOD   (1ULL<<44)
#define M6526_CNT   (1ULL<<45)

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

#define M6526_PORT_A (0)
#define M6526_PORT_B (1)
typedef uint8_t (*m6526_in_t)(int port_id, void* user_data);
typedef void (*m6526_out_t)(int port_id, uint8_t data, void* user_data);

typedef struct {
    m6526_in_t in_cb;
    m6526_out_t out_cb;
    void* user_data;
} m6526_desc_t;

typedef struct {
    uint8_t reg;        /* port register */
    uint8_t ddr;        /* data direction register */
    uint8_t inp;        /* input latch */
    uint8_t last_out;   /* last value to reduce call to out callback */
} m6526_port_t;

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

typedef struct {
    uint8_t imr;        /* interrupt mask */
    uint8_t imr1;       /* one cycle delay for imr updates */
    uint8_t icr;        /* interrupt control register */
    uint8_t pip_irq;    /* 1-cycle delay pipeline to request irq */
    bool flag;          /* last state of flag bit, to detect edge */
} m6526_int_t;

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

#define M6526_GET_DATA(p) ((uint8_t)(p>>16))
#define M6526_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
#define M6526_SET_ADDR(p,d) {p=((p&~0xF)|(d&0xF));}

extern void m6526_init(m6526_t* c, m6526_desc_t* desc);
extern void m6526_reset(m6526_t* c);
extern uint64_t m6526_iorq(m6526_t* c, uint64_t pins);
extern uint64_t m6526_tick(m6526_t* c, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
