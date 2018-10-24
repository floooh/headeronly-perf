#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z80PIO_M1       (1ULL<<24)      /* CPU Machine Cycle One, same as Z80_M1 */
#define Z80PIO_IORQ     (1ULL<<26)      /* IO Request from CPU, same as Z80_IORQ */
#define Z80PIO_RD       (1ULL<<27)      /* Read Cycle Status from CPU, same as Z80_RD */
#define Z80PIO_INT      (1ULL<<30)      /* Interrupt Request, same as Z80_INT */

#define Z80PIO_IEIO     (1ULL<<37)      /* combined Interrupt Enable In/Out (same as Z80_IEIO) */
#define Z80PIO_RETI     (1ULL<<38)      /* CPU has decoded a RETI instruction (same as Z80_RETI) */

#define Z80PIO_CE       (1ULL<<44)      /* Chip Enable */
#define Z80PIO_BASEL    (1ULL<<45)      /* Port A/B Select (inactive: A, active: B) */
#define Z80PIO_CDSEL    (1ULL<<46)      /* Control/Data Select (inactive: data, active: control) */
#define Z80PIO_ARDY     (1ULL<<47)      /* Port A Ready */
#define Z80PIO_BRDY     (1ULL<<48)      /* Port B Ready */
#define Z80PIO_ASTB     (1ULL<<49)      /* Port A Strobe */
#define Z80PIO_BSTB     (1ULL<<40)      /* Port B Strobe */

#define Z80PIO_PORT_A       (0)
#define Z80PIO_PORT_B       (1)
#define Z80PIO_NUM_PORTS    (2)

#define Z80PIO_MODE_OUTPUT           (0)
#define Z80PIO_MODE_INPUT            (1)
#define Z80PIO_MODE_BIDIRECTIONAL    (2)
#define Z80PIO_MODE_BITCONTROL       (3) /* only port A */

#define Z80PIO_INTCTRL_EI           (1<<7)
#define Z80PIO_INTCTRL_ANDOR        (1<<6)
#define Z80PIO_INTCTRL_HILO         (1<<5)
#define Z80PIO_INTCTRL_MASK_FOLLOWS (1<<4)

#define Z80PIO_INT_NEEDED (1<<0)
#define Z80PIO_INT_REQUESTED (1<<1)
#define Z80PIO_INT_SERVICING (1<<2)

typedef struct {
    uint8_t input;          /* 8-bit data input register */
    uint8_t output;         /* 8-bit data output register */
    uint8_t mode;           /* 2-bit mode control register (Z80PIO_MODE_*) */
    uint8_t io_select;      /* 8-bit input/output select register */
    uint8_t int_vector;     /* 8-bit interrupt vector */
    uint8_t int_control;    /* interrupt control word (Z80PIO_INTCTRL_*) */
    uint8_t int_mask;       /* 8-bit interrupt control mask */
    uint8_t int_state;      /* current interrupt handling state */
    bool int_enabled;       /* definitive interrupt enabled flag */
    bool expect_io_select;  /* next control word will be io_select */
    bool expect_int_mask;   /* next control word will be  mask */
    bool bctrl_match;       /* bitcontrol logic equation result */
} z80pio_port_t;

typedef uint8_t (*z80pio_in_t)(int port_id, void* user_data);
typedef void (*z80pio_out_t)(int port_id, uint8_t data, void* user_data);

typedef struct {
    z80pio_in_t in_cb;      /* port-input callback */
    z80pio_out_t out_cb;    /* port-output callback */
    void* user_data;        /* user-data handed to callbacks */
} z80pio_desc_t;

typedef struct {
    z80pio_port_t port[Z80PIO_NUM_PORTS];
    bool reset_active;
    z80pio_in_t in_cb;
    z80pio_out_t out_cb;
    void* user_data;
} z80pio_t;

#define Z80PIO_GET_DATA(p) ((uint8_t)(p>>16))
#define Z80PIO_SET_DATA(p,d) {p=((p&~0xFF0000)|((d&0xFF)<<16));}

extern void z80pio_init(z80pio_t* pio, z80pio_desc_t* desc);
extern void z80pio_reset(z80pio_t* pio);
extern uint64_t z80pio_iorq(z80pio_t* pio, uint64_t pins);
extern void z80pio_write_port(z80pio_t* pio, int port_id, uint8_t data);
extern uint64_t z80pio_int(z80pio_t* pio, uint64_t pins);

#ifdef __cplusplus
} /* extern "C" */
#endif
