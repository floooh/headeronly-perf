#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#define M6502_D0  (1ULL<<16)
#define M6502_D1  (1ULL<<17)
#define M6502_D2  (1ULL<<18)
#define M6502_D3  (1ULL<<19)
#define M6502_D4  (1ULL<<20)
#define M6502_D5  (1ULL<<21)
#define M6502_D6  (1ULL<<22)
#define M6502_D7  (1ULL<<23)

#define M6502_RW    (1ULL<<24)
#define M6502_SYNC  (1ULL<<25)
#define M6502_IRQ   (1ULL<<26)
#define M6502_NMI   (1ULL<<27)
#define M6502_RDY   (1ULL<<28)
#define M6510_AEC   (1ULL<<29)

#define M6502_PIN_MASK (0xFFFFFFFF)

#define M6502_CF (1<<0)   /* carry */
#define M6502_ZF (1<<1)   /* zero */
#define M6502_IF (1<<2)   /* IRQ disable */
#define M6502_DF (1<<3)   /* decimal mode */
#define M6502_BF (1<<4)   /* BRK command */
#define M6502_XF (1<<5)   /* unused */
#define M6502_VF (1<<6)   /* overflow */
#define M6502_NF (1<<7)   /* negative */

#define M6502_MAX_NUM_TRAPS (8)

typedef uint64_t (*m6502_tick_t)(uint64_t pins, void* user_data);
typedef void (*m6510_out_t)(uint8_t data, void* user_data);
typedef uint8_t (*m6510_in_t)(void* user_data);

typedef struct {
    m6502_tick_t tick_cb;   /* the CPU tick callback */
    bool bcd_disabled;      /* set to true if BCD mode is disabled */
    m6510_in_t in_cb;       /* optional port IO input callback (only on m6510) */
    m6510_out_t out_cb;     /* optional port IO output callback (only on m6510) */
    uint8_t m6510_io_pullup;    /* IO port bits that are 1 when reading */
    uint8_t m6510_io_floating;  /* unconnected IO port pins */
    void* user_data;        /* optional user-data for callbacks */
} m6502_desc_t;

typedef struct {
    uint64_t PINS;
    uint8_t A,X,Y,S,P;      /* 8-bit registers */
    uint16_t PC;            /* 16-bit program counter */
    uint8_t pi;
    bool bcd_enabled;       /* this is actually not mutable but needed when ticking */
} m6502_state_t;

typedef struct {
    m6502_state_t state;
    m6502_tick_t tick;
    void* user_data;
    m6510_in_t in_cb;
    m6510_out_t out_cb;
    uint8_t io_ddr;     /* 1: output, 0: input */
    uint8_t io_port;
    uint8_t io_pullup;
    uint8_t io_floating;
    uint8_t io_drive;
    bool trap_valid[M6502_MAX_NUM_TRAPS];
    uint16_t trap_addr[M6502_MAX_NUM_TRAPS];
    int trap_id;        /* index of trap hit (-1 if no trap) */
} m6502_t;

extern void m6502_init(m6502_t* cpu, m6502_desc_t* desc);
extern void m6502_reset(m6502_t* cpu);
extern void m6502_set_trap(m6502_t* cpu, int trap_id, uint16_t addr);
extern void m6502_clear_trap(m6502_t* cpu, int trap_id);
extern bool m6502_has_trap(m6502_t* cpu, int trap_id);
extern uint32_t m6502_exec(m6502_t* cpu, uint32_t ticks);
extern uint64_t m6510_iorq(m6502_t* cpu, uint64_t pins);

#define M6502_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
#define M6502_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
#define M6502_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
#define M6502_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
#define M6502_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))

#define M6510_CHECK_IO(p) ((p&0xFFFEULL)==0)

#ifdef __cplusplus
} /* extern "C" */
#endif
