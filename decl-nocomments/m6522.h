#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M6522_RW    (1ULL<<24)      /* RW pin is on same location as M6502 RW pin */
#define M6522_CS1   (1ULL<<40)      /* chip-select 1, to select: CS1 high, CS2 low */
#define M6522_CS2   (1ULL<<41)      /* chip-select 2 */
#define M6522_CA1   (1ULL<<42)      /* peripheral A control 1 */
#define M6522_CA2   (1ULL<<43)      /* peripheral A control 2 */
#define M6522_CB1   (1ULL<<44)      /* peripheral B control 1 */
#define M6522_CB2   (1ULL<<45)      /* peripheral B control 2 */

#define M6522_PA0   (1ULL<<48)
#define M6522_PA1   (1ULL<<49)
#define M6522_PA2   (1ULL<<50)
#define M6522_PA3   (1ULL<<51)
#define M6522_PA4   (1ULL<<52)
#define M6522_PA5   (1ULL<<53)
#define M6522_PA6   (1ULL<<54)
#define M6522_PA7   (1ULL<<55)

#define M6522_PB0   (1ULL<<56)
#define M6522_PB1   (1ULL<<57)
#define M6522_PB2   (1ULL<<58)
#define M6522_PB3   (1ULL<<59)
#define M6522_PB4   (1ULL<<60)
#define M6522_PB5   (1ULL<<61)
#define M6522_PB6   (1ULL<<62)
#define M6522_PB7   (1ULL<<63)

#define M6522_RS0   (1ULL<<0)
#define M6522_RS1   (1ULL<<1)
#define M6522_RS2   (1ULL<<2)
#define M6522_RS3   (1ULL<<3)
#define M6522_RS    (M6522_RS3|M6522_RS2|M6522_RS1|M6522_RS0)

#define M6522_D0    (1ULL<<16)
#define M6522_D1    (1ULL<<17)
#define M6522_D2    (1ULL<<18)
#define M6522_D3    (1ULL<<19)
#define M6522_D4    (1ULL<<20)
#define M6522_D5    (1ULL<<21)
#define M6522_D6    (1ULL<<22)
#define M6522_D7    (1ULL<<23)

#define M6522_REG_RB        (0)     /* input/output register B */
#define M6522_REG_RA        (1)     /* input/output register A */
#define M6522_REG_DDRB      (2)     /* data direction B */
#define M6522_REG_DDRA      (3)     /* data direction A */
#define M6522_REG_T1CL      (4)     /* T1 low-order latch / counter */
#define M6522_REG_T1CH      (5)     /* T1 high-order counter */
#define M6522_REG_T1LL      (6)     /* T1 low-order latches */
#define M6522_REG_T1LH      (7)     /* T1 high-order latches */
#define M6522_REG_T2CL      (8)     /* T2 low-order latch / counter */
#define M6522_REG_T2CH      (9)     /* T2 high-order counter */
#define M6522_REG_SR        (10)    /* shift register */
#define M6522_REG_ACR       (11)    /* auxiliary control register */
#define M6522_REG_PCR       (12)    /* peripheral control register */
#define M6522_REG_IFR       (13)    /* interrupt flag register */
#define M6522_REG_IER       (14)    /* interrupt enable register */
#define M6522_REG_RA_NOH    (15)    /* input/output A without handshake */
#define M6522_NUM_REGS      (16)

#define M6522_NUM_PORTS (2)

#define M6522_ACR_LATCH_A           (1<<0)
#define M6522_ACR_LATCH_B           (1<<1)
#define M6522_ACR_SHIFT_DISABLE     (0)
#define M6522_ACR_T2_COUNT          (1<<5)
#define M6522_ACR_T1_CONT_INT       (1<<6)
#define M6522_ACR_T1_PB7            (1<<7)

#define M6522_PORT_A (0)
#define M6522_PORT_B (1)
typedef uint8_t (*m6522_in_t)(int port_id, void* user_data);
typedef void (*m6522_out_t)(int port_id, uint8_t data, void* user_data);

typedef struct {
    m6522_in_t in_cb;
    m6522_out_t out_cb;
    void* user_data;
} m6522_desc_t;

typedef struct {
    uint8_t out_b, in_b, ddr_b;
    uint8_t out_a, in_a, ddr_a;
    uint8_t acr, pcr;
    uint8_t t1_pb7;         /* timer 1 toggle bit (masked into port B bit 7) */
    uint8_t t1ll, t1lh;     /* timer 1 latch low, high */
    uint8_t t2ll, t2lh;     /* timer 2 latch low, high */
    uint16_t t1;            /* timer1 counter */
    uint16_t t2;            /* timer2 counter */
    bool t1_active;         /* timer1 active */
    bool t2_active;         /* timer2 active */
    m6522_in_t in_cb;
    m6522_out_t out_cb;
    void* user_data;
} m6522_t;

#define M6522_GET_DATA(p) ((uint8_t)(p>>16))
#define M6522_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}
#define M6522_SET_ADDR(p,d) {p=((p&~0xF)|(d&0xF));}

extern void m6522_init(m6522_t* m6522, m6522_desc_t* desc);
extern void m6522_reset(m6522_t* m6522);
extern uint64_t m6522_iorq(m6522_t* m6522, uint64_t pins);
extern void m6522_tick(m6522_t* m6522);

#ifdef __cplusplus
} /* extern "C" */
#endif
