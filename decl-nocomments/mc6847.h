#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MC6847_A0   (1ULL<<0)
#define MC6847_A1   (1ULL<<1)
#define MC6847_A2   (1ULL<<2)
#define MC6847_A3   (1ULL<<3)
#define MC6847_A4   (1ULL<<4)
#define MC6847_A5   (1ULL<<5)
#define MC6847_A6   (1ULL<<6)
#define MC6847_A7   (1ULL<<7)
#define MC6847_A8   (1ULL<<8)
#define MC6847_A9   (1ULL<<9)
#define MC6847_A10  (1ULL<<10)
#define MC6847_A11  (1ULL<<11)
#define MC6847_A12  (1ULL<<12)

#define MC6847_D0   (1ULL<<16)
#define MC6847_D1   (1ULL<<17)
#define MC6847_D2   (1ULL<<18)
#define MC6847_D3   (1ULL<<19)
#define MC6847_D4   (1ULL<<20)
#define MC6847_D5   (1ULL<<21)
#define MC6847_D6   (1ULL<<22)
#define MC6847_D7   (1ULL<<23)

#define MC6847_FS   (1ULL<<40)      /* field sync */
#define MC6847_HS   (1ULL<<41)      /* horizontal sync */
#define MC6847_RP   (1ULL<<42)      /* row preset */

#define MC6847_AG       (1ULL<<43)      /* graphics mode enable */
#define MC6847_AS       (1ULL<<44)      /* semi-graphics mode enable */
#define MC6847_INTEXT   (1ULL<<45)      /* internal/external select */
#define MC6847_INV      (1ULL<<46)      /* invert enable */
#define MC6847_GM0      (1ULL<<47)      /* graphics mode select 0 */
#define MC6847_GM1      (1ULL<<48)      /* graphics mode select 1 */
#define MC6847_GM2      (1ULL<<49)      /* graphics mode select 2 */
#define MC6847_CSS      (1ULL<<50)      /* color select pin */

#define MC6847_GET_ADDR(p) ((uint16_t)(p&0xFFFFULL))
#define MC6847_SET_ADDR(p,a) {p=((p&~0xFFFFULL)|((a)&0xFFFFULL));}
#define MC6847_GET_DATA(p) ((uint8_t)((p&0xFF0000ULL)>>16))
#define MC6847_SET_DATA(p,d) {p=((p&~0xFF0000ULL)|(((d)<<16)&0xFF0000ULL));}

#define MC6847_VBLANK_LINES         (13)    /* 13 lines vblank at top of screen */
#define MC6847_TOP_BORDER_LINES     (25)    /* 25 lines top border */
#define MC6847_DISPLAY_LINES        (192)   /* 192 lines visible display area */
#define MC6847_BOTTOM_BORDER_LINES  (26)    /* 26 lines bottom border */
#define MC6847_VRETRACE_LINES       (6)     /* 6 'lines' for vertical retrace */
#define MC6847_ALL_LINES            (262)   /* all of the above */
#define MC6847_DISPLAY_START        (MC6847_VBLANK_LINES+MC6847_TOP_BORDER_LINES)
#define MC6847_DISPLAY_END          (MC6847_DISPLAY_START+MC6847_DISPLAY_LINES)
#define MC6847_BOTTOM_BORDER_END    (MC6847_DISPLAY_END+MC6847_BOTTOM_BORDER_LINES)
#define MC6847_FSYNC_START          (MC6847_DISPLAY_END)

#define MC6847_DISPLAY_WIDTH (320)
#define MC6847_DISPLAY_HEIGHT (MC6847_TOP_BORDER_LINES+MC6847_DISPLAY_LINES+MC6847_BOTTOM_BORDER_LINES)

#define MC6847_IMAGE_WIDTH (256)
#define MC6847_IMAGE_HEIGHT (192)

#define MC6847_BORDER_PIXELS ((MC6847_DISPLAY_WIDTH-MC6847_IMAGE_WIDTH)/2)

#define MC6847_TICK_HZ (3579545)

#define MC6847_FIXEDPOINT_SCALE (16)

typedef uint64_t (*mc6847_fetch_t)(uint64_t pins, void* user_data);

typedef struct {
    int tick_hz;
    uint32_t* rgba8_buffer;
    uint32_t rgba8_buffer_size;
    mc6847_fetch_t fetch_cb;
    void* user_data;
} mc6847_desc_t;

typedef struct {
    uint64_t pins;
    uint64_t on;
    uint64_t off;
    uint32_t palette[8];
    uint32_t black;
    uint32_t alnum_green;
    uint32_t alnum_dark_green;
    uint32_t alnum_orange;
    uint32_t alnum_dark_orange;
    int h_count;
    int h_sync_start;
    int h_sync_end;
    int h_period;
    int l_count;
    mc6847_fetch_t fetch_cb;
    void* user_data;
    uint32_t* rgba8_buffer;
} mc6847_t;

extern void mc6847_init(mc6847_t* vdg, mc6847_desc_t* desc);
extern void mc6847_reset(mc6847_t* vdg);
extern void mc6847_ctrl(mc6847_t* vdg, uint64_t pins, uint64_t mask);
extern void mc6847_tick(mc6847_t* vdg);

#ifdef __cplusplus
} /* extern "C" */
#endif
