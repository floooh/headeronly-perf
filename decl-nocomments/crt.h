#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CRT_PAL,
    CRT_NTSC
} crt_std;

#define CRT_PAL_H_DISPLAY_START (6)
#define CRT_PAL_V_DISPLAY_START (5)
#define CRT_PAL_V_DISPLAY_END   (6+304)
#define CRT_PAL_H_DISPLAY_END   (6+52)

typedef struct {
    int video_standard;     /* CRT_PAL or CRT_NTSC */
    int vis_x0, vis_x1, vis_y0, vis_y1;     /* visible region */
    int pos_x, pos_y;       /* current beam position in visible region */
    bool visible;           /* true if beam is current in visible region */
    int h_pos;      /* current horizontal position (0..63) */
    int v_pos;      /* current vertical position (0..312) */
    bool h_sync;    /* last state of hsync, to detect raising edge trigger */
    bool v_sync;    /* last state of vsync, to detect raising edge trigger */
    bool h_blank;   /* horizontal blanking active (retrace + porches) */
    bool v_blank;   /* vertical blanking active */
    int h_retrace;  /* horizontal retrace counter, this is >0 during horizontal retrace */
    int v_retrace;  /* vertical retrace counter, this is >0 during vertical retrace */
} crt_t;

extern void crt_init(crt_t* crt, crt_std video_std, int vis_x, int vis_y, int vis_w, int vis_h);
extern void crt_reset(crt_t* crt);
extern bool crt_tick(crt_t* crt, bool hsync, bool vsync);

#ifdef __cplusplus
} /* extern "C" */
#endif
