#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z80CTC_M1       (1ULL<<24)   /* CPU Machine Cycle One (same as Z80_M1) */
#define Z80CTC_IORQ     (1ULL<<26)   /* CPU IO Request (same as Z80_IORQ) */
#define Z80CTC_RD       (1ULL<<27)   /* CPU Read Cycle Status (same as Z80_RD) */
#define Z80CTC_INT      (1ULL<<30)   /* Interrupt Request (same as Z80_INT) */
#define Z80CTC_RESET    (1ULL<<31)   /* put CTC into reset state (same as Z80_RESET) */

#define Z80CTC_IEIO     (1ULL<<37)   /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80CTC_RETI     (1ULL<<38)   /* CPU has decoded a RETI instruction (same as Z80_RETI) */

#define Z80CTC_CE       (1ULL<<44)   /* Chip Enable */
#define Z80CTC_CS0      (1ULL<<45)   /* Channel Select Bit 0 */
#define Z80CTC_CS1      (1ULL<<46)   /* Channel Select Bit 1 */
#define Z80CTC_CLKTRG0  (1ULL<<47)   /* Clock/Timer Trigger 0 */
#define Z80CTC_CLKTRG1  (1ULL<<48)   /* Clock/Timer Trigger 1 */
#define Z80CTC_CLKTRG2  (1ULL<<49)   /* Clock/Timer Trigger 2 */
#define Z80CTC_CLKTRG3  (1ULL<<50)   /* Clock/Timer Trigger 3 */
#define Z80CTC_ZCTO0    (1ULL<<51)   /* Zero Count/Timeout 0 */
#define Z80CTC_ZCTO1    (1ULL<<52)   /* Zero Count/Timeout 1 */
#define Z80CTC_ZCTO2    (1ULL<<53)   /* Zero Count/Timeout 2 */

#define Z80CTC_CTRL_EI              (1<<7)  /* 1: interrupt enabled, 0: interrupt disabled */

#define Z80CTC_CTRL_MODE            (1<<6)  /* 1: counter mode, 0: timer mode */
#define Z80CTC_CTRL_MODE_COUNTER    (1<<6)
#define Z80CTC_CTRL_MODE_TIMER      (0)

#define Z80CTC_CTRL_PRESCALER       (1<<5)  /* 1: prescale value 256, 0: prescaler value 16 */
#define Z80CTC_CTRL_PRESCALER_256   (1<<5)
#define Z80CTC_CTRL_PRESCALER_16    (0)

#define Z80CTC_CTRL_EDGE            (1<<4)  /* 1: rising edge, 0: falling edge */
#define Z80CTC_CTRL_EDGE_RISING     (1<<4)
#define Z80CTC_CTRL_EDGE_FALLING    (0)

#define Z80CTC_CTRL_TRIGGER         (1<<3)  /* 1: CLK/TRG pulse starts timer, 0: trigger when time constant loaded */
#define Z80CTC_CTRL_TRIGGER_WAIT    (1<<3)
#define Z80CTC_CTRL_TRIGGER_AUTO    (0)

#define Z80CTC_CTRL_CONST_FOLLOWS   (1<<2)  /* 1: time constant follows, 0: no time constant follows */
#define Z80CTC_CTRL_RESET           (1<<1)  /* 1: software reset, 0: continue operation */
#define Z80CTC_CTRL_CONTROL         (1<<0)  /* 1: control, 0: vector */
#define Z80CTC_CTRL_VECTOR          (0)

#define Z80CTC_INT_NEEDED           (1<<0)  /* interrupt request needed */
#define Z80CTC_INT_REQUESTED        (1<<1)  /* interrupt request issued, waiting for ack from CPU */
#define Z80CTC_INT_SERVICING        (1<<2)  /* interrupt was acknowledged, now servicing */

typedef struct {
    uint8_t control;        /* Z80CTC_CTRL_xxx */
    uint8_t constant;
    uint8_t down_counter;
    uint8_t prescaler;
    uint8_t int_vector;
    bool trigger_edge;
    bool waiting_for_trigger;
    bool ext_trigger;
    uint8_t prescaler_mask;
    uint8_t int_state;
} z80ctc_channel_t;

#define Z80CTC_NUM_CHANNELS (4)

typedef struct {
    z80ctc_channel_t chn[Z80CTC_NUM_CHANNELS];
} z80ctc_t;

#define Z80CTC_GET_DATA(p) ((uint8_t)(p>>16))
#define Z80CTC_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

extern void z80ctc_init(z80ctc_t* ctc);
extern void z80ctc_reset(z80ctc_t* ctc);
extern uint64_t z80ctc_iorq(z80ctc_t* ctc, uint64_t pins);

extern uint64_t _z80ctc_counter_zero(z80ctc_channel_t* chn, uint64_t pins, int chn_id);
extern uint64_t _z80ctc_active_edge(z80ctc_channel_t* chn, uint64_t pins, int chn_id);
extern uint64_t z80ctc_tick(z80ctc_t* ctc, uint64_t pins);
extern uint64_t z80ctc_int(z80ctc_t* ctc, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
