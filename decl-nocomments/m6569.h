#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define M6569_A0    (1ULL<<0)
#define M6569_A1    (1ULL<<1)
#define M6569_A2    (1ULL<<2)
#define M6569_A3    (1ULL<<3)
#define M6569_A4    (1ULL<<4)
#define M6569_A5    (1ULL<<5)
#define M6569_A6    (1ULL<<6)
#define M6569_A7    (1ULL<<7)
#define M6569_A8    (1ULL<<8)
#define M6569_A9    (1ULL<<9)
#define M6569_A10   (1ULL<<10)
#define M6569_A11   (1ULL<<11)
#define M6569_A12   (1ULL<<12)
#define M6569_A13   (1ULL<<13)

#define M6569_D0    (1ULL<<16)
#define M6569_D1    (1ULL<<17)
#define M6569_D2    (1ULL<<18)
#define M6569_D3    (1ULL<<19)
#define M6569_D4    (1ULL<<20)
#define M6569_D5    (1ULL<<21)
#define M6569_D6    (1ULL<<22)
#define M6569_D7    (1ULL<<23)

#define M6569_RW    (1ULL<<24)      /* shared with m6502 CPU */
#define M6569_IRQ   (1ULL<<26)      /* shared with m6502 CPU */
#define M6569_BA    (1ULL<<28)      /* shared with m6502 RDY */
#define M6569_AEC   (1ULL<<29)      /* shared with m6510 AEC */

#define M6569_CS    (1ULL<<40)

#define M6569_NUM_REGS (64)
#define M6569_REG_MASK (M6569_NUM_REGS-1)
#define M6569_NUM_MOBS (8)

#define M6569_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
#define M6569_SET_DATA(p,d) {p=(((p)&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

typedef uint16_t (*m6569_fetch_t)(uint16_t addr, void* user_data);

typedef struct {
    uint32_t* rgba8_buffer;
    uint32_t rgba8_buffer_size;
    uint16_t vis_x, vis_y, vis_w, vis_h;
    m6569_fetch_t fetch_cb;
    void* user_data;
} m6569_desc_t;

typedef struct {
    union {
        uint8_t regs[M6569_NUM_REGS];
        struct {
            uint8_t mxy[M6569_NUM_MOBS][2];     /* sprite X/Y coords */
            uint8_t mx8;                        /* x coordinate MSBs */
            uint8_t ctrl_1;                     /* control register 1 */
            uint8_t raster;                     /* raster counter */
            uint8_t lightpen_xy[2];             /* lightpen coords */
            uint8_t me;                         /* sprite enabled bits */
            uint8_t ctrl_2;                     /* control register 2 */
            uint8_t mye;                        /* sprite Y expansion */
            uint8_t mem_ptrs;                   /* memory pointers */
            uint8_t int_latch;                  /* interrupt latch */
            uint8_t int_mask;                   /* interrupt-enabled mask */
            uint8_t mdp;                        /* sprite data priority bits */
            uint8_t mmc;                        /* sprite multicolor bits */
            uint8_t mxe;                        /* sprite X expansion */
            uint8_t mcm;                        /* sprite-sprite collision bits */
            uint8_t mcd;                        /* sprite-data collision bits */
            uint8_t ec;                         /* border color */
            uint8_t bc[4];                      /* background colors */
            uint8_t mm[2];                      /* sprite multicolor 0 */
            uint8_t mc[8];                      /* sprite colors */
            uint8_t unused[17];                 /* not writable, return 0xFF on read */
        };
    };
} _m6569_registers_t;

#define M6569_CTRL1_RST8    (1<<7)
#define M6569_CTRL1_ECM     (1<<6)
#define M6569_CTRL1_BMM     (1<<5)
#define M6569_CTRL1_DEN     (1<<4)
#define M6569_CTRL1_RSEL    (1<<3)
#define M6569_CTRL1_YSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6569_CTRL2_RES     (1<<5)
#define M6569_CTRL2_MCM     (1<<4)
#define M6569_CTRL2_CSEL    (1<<3)
#define M6569_CTRL2_XSCROLL ((1<<2)|(1<<1)|(1<<0))
#define M6569_INT_IRQ       (1<<7)      /* int_latch: interrupt requested */
#define M6569_INT_ILP       (1<<3)      /* int_latch: lightpen interrupt */
#define M6569_INT_IMMC      (1<<2)      /* int_latch: mob/mob collision interrupt */
#define M6569_INT_IMBC      (1<<1)      /* int_latch: mob/bitmap collision interrupt */
#define M6569_INT_IRST      (1<<0)      /* int_latch: raster interrupt */
#define M6569_INT_ELP       (1<<3)      /* int_mask: lightpen interrupt enabled */
#define M6569_INT_EMMC      (1<<2)      /* int_mask: mob/mob collision interrupt enabled */
#define M6569_INT_EMBC      (1<<1)      /* int_mask: mob/bitmap collision interrupt enabled */
#define M6569_INT_ERST      (1<<0)      /* int_mask: raster interrupt enabled */

typedef struct {
    uint16_t h_count;
    uint16_t v_count;
    uint16_t v_irqline;     /* raster interrupt line, updated when ctrl_1 or raster is written */
    uint16_t sh_count;      /* separate counter for sprite, reset at h_count=55 */
    uint16_t vc;            /* 10-bit video counter */
    uint16_t vc_base;       /* 10-bit video counter base */
    uint8_t rc;             /* 3-bit raster counter */
    bool display_state;             /* true: in display state, false: in idle state */
    bool badline;                   /* true when the badline state is active */
    bool frame_badlines_enabled;    /* true when badlines are enabled in frame */
} _m6569_raster_unit_t;

typedef struct {
    uint16_t c_addr_or;     /* OR-mask for c-accesses, computed from mem_ptrs */
    uint16_t g_addr_and;    /* AND-mask for g-accesses, computed from mem_ptrs */
    uint16_t g_addr_or;     /* OR-mask for g-accesses, computed from ECM bit */
    uint16_t i_addr;        /* address for i-accesses, 0x3FFF or 0x39FF (if ECM bit set) */
    uint16_t p_addr_or;     /* OR-mask for p-accesses */
    m6569_fetch_t fetch_cb; /* memory-fetch callback */
    void* user_data;        /* optional user-data for fetch callback */
} _m6569_memory_unit_t;

typedef struct {
    uint8_t vmli;           /* 6-bit 'vmli' video-matrix line buffer index */
    uint16_t line[64];      /* 40x 8+4 bits line buffer (64 items because vmli is a 6-bit ctr) */
} _m6569_video_matrix_t;

typedef struct {
    uint16_t left, right, top, bottom;
    bool main;          /* main border flip-flop */
    bool vert;          /* vertical border flip flop */
    uint8_t bc_index;   /* border color as palette index (not used, but may be usefil for outside code) */
    uint32_t bc_rgba8;  /* border color as RGBA8, udpated when border color register is updated */
} _m6569_border_unit_t;

typedef struct {
    uint16_t x, y;              /* bream pos reset on crt_retrace_h/crt_retrace_v zero */
    uint16_t vis_x0, vis_y0, vis_x1, vis_y1;  /* the visible area */
    uint16_t vis_w, vis_h;      /* width of visible area */
    uint32_t* rgba8_buffer;
} _m6569_crt_t;

typedef struct {
    uint8_t mode;               /* display mode 0..7 precomputed from ECM/BMM/MCM bits */
    bool enabled;               /* true while g_accesses are happening */
    uint8_t count;              /* counts from 0..8 */
    uint8_t shift;              /* current pixel shifter */
    uint8_t outp;               /* current output byte (bit 7) */
    uint8_t outp2;              /* current output byte at half frequency (bits 7 and 6) */
    uint16_t c_data;            /* loaded from video matrix line buffer */
    uint8_t bg_index[4];        /* background color as palette index (not used, but may be useful for outside code) */
    uint32_t bg_rgba8[4];       /* background colors as RGBA8 */
} _m6569_graphics_unit_t;

typedef struct {
    uint8_t h_first;            /* first horizontal visible tick */
    uint8_t h_last;             /* last horizontal visible tick */
    uint8_t h_offset;           /* x-offset within 8-pixel raster */

    uint8_t p_data;             /* the byte read by p_access memory fetch */
    
    bool dma_enabled;           /* sprite dma is enabled */
    bool disp_enabled;          /* sprite display is enabled */
    bool expand;                /* expand flip-flop */
    uint8_t mc;                 /* 6-bit mob-data-counter */
    uint8_t mc_base;            /* 6-bit mob-data-counter base */
    uint8_t delay_count;        /* 0..7 delay pixels */
    uint8_t outp2_count;        /* outp2 is updated when bit 0 is on */
    uint8_t xexp_count;         /* if x stretched, only shift every second pixel tick */
    uint32_t shift;             /* 24-bit shift register */
    uint32_t outp;              /* current shifter output (bit 31) */
    uint32_t outp2;             /* current shifter output at half frequency (bits 31 and 30) */
    uint32_t colors[4];         /* 0: unused, 1: multicolor0, 2: main color, 3: multicolor1
                                   the alpha channel is cleared and used as bitmask for sprites
                                   which produced a color
                                */
} _m6569_sprite_unit_t;

typedef struct {
    bool debug_vis;             /* toggle this to switch debug visualization on/off */
    _m6569_registers_t reg;
    _m6569_raster_unit_t rs;
    _m6569_crt_t crt;
    _m6569_border_unit_t brd;
    _m6569_memory_unit_t mem;
    _m6569_video_matrix_t vm;
    _m6569_graphics_unit_t gunit;
    _m6569_sprite_unit_t sunit[8];
} m6569_t;

extern void m6569_init(m6569_t* vic, m6569_desc_t* desc);
extern void m6569_reset(m6569_t* vic);
extern void m6569_display_size(m6569_t* vic, int* out_width, int* out_height);
extern uint64_t m6569_iorq(m6569_t* vic, uint64_t pins);
extern uint64_t m6569_tick(m6569_t* vic, uint64_t pins);
extern uint32_t m6569_color(int i);

#ifdef __cplusplus
} /* extern "C" */
#endif
