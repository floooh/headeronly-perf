#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t (*z80_tick_t)(int num_ticks, uint64_t pins, void* user_data);

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

#define Z80_D0  (1ULL<<16)
#define Z80_D1  (1ULL<<17)
#define Z80_D2  (1ULL<<18)
#define Z80_D3  (1ULL<<19)
#define Z80_D4  (1ULL<<20)
#define Z80_D5  (1ULL<<21)
#define Z80_D6  (1ULL<<22)
#define Z80_D7  (1ULL<<23)

#define  Z80_M1    (1ULL<<24)       /* machine cycle 1 */
#define  Z80_MREQ  (1ULL<<25)       /* memory request */
#define  Z80_IORQ  (1ULL<<26)       /* input/output request */
#define  Z80_RD    (1ULL<<27)       /* read */
#define  Z80_WR    (1ULL<<28)       /* write */
#define  Z80_CTRL_MASK (Z80_M1|Z80_MREQ|Z80_IORQ|Z80_RD|Z80_WR)

#define  Z80_HALT  (1ULL<<29)       /* halt state */
#define  Z80_INT   (1ULL<<30)       /* interrupt request */
#define  Z80_NMI   (1ULL<<31)       /* non-maskable interrupt */
#define  Z80_BUSREQ (1ULL<<32)      /* bus request */
#define  Z80_BUSACK (1ULL<<33)      /* bus acknowledge */

#define Z80_WAIT0   (1ULL<<34)
#define Z80_WAIT1   (1ULL<<35)
#define Z80_WAIT2   (1ULL<<36)
#define Z80_WAIT_SHIFT (34)
#define Z80_WAIT_MASK (Z80_WAIT0|Z80_WAIT1|Z80_WAIT2)

#define Z80_IEIO    (1ULL<<37)      /* unified daisy chain 'Interrupt Enable In+Out' */
#define Z80_RETI    (1ULL<<38)      /* cpu has decoded a RETI instruction */

#define Z80_PIN_MASK ((1ULL<<40)-1)

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

typedef struct {
    z80_tick_t tick_cb;
    void* user_data;
} z80_desc_t;

typedef struct {
    z80_tick_t tick;
    uint64_t bc_de_hl_fa;   /* B:63..56 C:55..48 D:47..40 E:39..32 H:31..24 L:23..16: F:15..8, A:7..0 */
    uint64_t bc_de_hl_fa_;
    uint64_t wz_ix_iy_sp;
    uint64_t im_ir_pc_bits;
    uint64_t pins;
    void* user_data;
    int trap_id;
    uint64_t trap_addr;
} z80_t;

extern void z80_init(z80_t* cpu, z80_desc_t* desc);
extern void z80_reset(z80_t* cpu);
extern void z80_set_trap(z80_t* cpu, int trap_id, uint16_t addr);
extern void z80_clear_trap(z80_t* cpu, int trap_id);
extern bool z80_has_trap(z80_t* cpu, int trap_id);
extern uint32_t z80_exec(z80_t* cpu, uint32_t ticks);
extern bool z80_opdone(z80_t* cpu);

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

#define Z80_DAISYCHAIN_BEGIN(pins) if (pins&Z80_M1) { pins|=Z80_IEIO;
#define Z80_DAISYCHAIN_END(pins) pins&=~Z80_RETI; }
#define Z80_MAKE_PINS(ctrl, addr, data) ((ctrl)|(((data)<<16)&0xFF0000ULL)|((addr)&0xFFFFULL))
#define Z80_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
#define Z80_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
#define Z80_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
#define Z80_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}
#define Z80_GET_WAIT(p) ((p&Z80_WAIT_MASK)>>Z80_WAIT_SHIFT)
#define Z80_SET_WAIT(p,w) {p=((p&~Z80_WAIT_MASK)|((((uint64_t)w)<<Z80_WAIT_SHIFT)&Z80_WAIT_MASK));}

#ifdef __cplusplus
} /* extern "C" */
#endif
