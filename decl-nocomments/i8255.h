#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I8255_CS    (1ULL<<40)     /* chip-select, i8255 responds to RW/WR when this is active */
#define I8255_RD    (1ULL<<41)     /* read from PPI */
#define I8255_WR    (1ULL<<42)     /* write to PPI */
#define I8255_A0    (1ULL<<43)     /* address bit 0 */
#define I8255_A1    (1ULL<<44)     /* address bit 1 */

#define I8255_D0  (1ULL<<16)
#define I8255_D1  (1ULL<<17)
#define I8255_D2  (1ULL<<18)
#define I8255_D3  (1ULL<<19)
#define I8255_D4  (1ULL<<20)
#define I8255_D5  (1ULL<<21)
#define I8255_D6  (1ULL<<22)
#define I8255_D7  (1ULL<<23)

#define I8255_PORT_A    (0)
#define I8255_PORT_B    (1)
#define I8255_PORT_C    (2)
#define I8255_NUM_PORTS (3)

#define I8255_CTRL_CONTROL               (1<<7)
#define I8255_CTRL_CONTROL_MODE          (1<<7)
#define I8255_CTRL_CONTROL_BIT           (0)
#define I8255_CTRL_CLO              (1<<0)
#define I8255_CTRL_CLO_INPUT        (1<<0)
#define I8255_CTRL_CLO_OUTPUT       (0)
#define I8255_CTRL_B                (1<<1)
#define I8255_CTRL_B_INPUT          (1<<1)
#define I8255_CTRL_B_OUTPUT         (0)
#define I8255_CTRL_BCLO_MODE        (1<<2)
#define I8255_CTRL_BCLO_MODE_0      (0)
#define I8255_CTRL_BCLO_MIDE_1      (1<<2)
#define I8255_CTRL_CHI              (1<<3)
#define I8255_CTRL_CHI_INPUT        (1<<3)
#define I8255_CTRL_CHI_OUTPUT       (0)
#define I8255_CTRL_A                (1<<4)
#define I8255_CTRL_A_INPUT          (1<<4)
#define I8255_CTRL_A_OUTPUT         (0)
#define I8255_CTRL_ACHI_MODE        ((1<<6)|(1<<5))
#define I8255_CTRL_ACHI_MODE_0      (0)
#define I8255_CTRL_ACHI_MODE_1      (1<<5)
#define I8255_CTRL_BIT              (1<<0)
#define I8255_CTRL_BIT_SET          (1<<0)
#define I8255_CTRL_BIT_RESET        (0)

typedef uint8_t (*i8255_in_t)(int port_id, void* user_data);
typedef uint64_t (*i8255_out_t)(int port_id, uint64_t pins, uint8_t data, void* user_data);

typedef struct {
    i8255_in_t in_cb;       /* port-input callback */
    i8255_out_t out_cb;     /* port-output callback */
    void* user_data;        /* optional user-data for callbacks */
} i8255_desc_t;

typedef struct {
    uint8_t output[I8255_NUM_PORTS];
    uint8_t control;
    uint8_t dbg_input[I8255_NUM_PORTS];
    i8255_in_t in_cb;
    i8255_out_t out_cb;
    void* user_data;
} i8255_t;

#define I8255_GET_DATA(p) ((uint8_t)(p>>16))
#define I8255_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

extern void i8255_init(i8255_t* ppi, i8255_desc_t* desc);
extern void i8255_reset(i8255_t* ppi);
extern uint64_t i8255_iorq(i8255_t* ppi, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
