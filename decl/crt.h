#pragma once
/*#
    # crt.h

    PAL/NTSC CRT helper class.

    Do this:
    ~~~C
    #define CHIPS_IMPL
    ~~~
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
    ~~~C
    CHIPS_ASSERT(c)
    ~~~
        your own assert macro (default: assert(c))

    ## Overview

    The crt.h header provides timing information for a PAL or NTSC Cathode
    Ray Tube.
    
    The user code provides initializes a crt_t instance with the video
    standard and a visible region, calls the tick function at 1 microsecond
    steps, and provides the state of HSYNC and VSYNC.

    In return the crt_t instance provides a flag whether the beam
    is currently inside the visible area, and the current beam position
    inside the visible area.

    !!! Note FIXME 
        only PAL standard is currently implemented!

    ## Links

    - https://en.wikipedia.org/wiki/Horizontal_blanking_interval
    - http://www.batsocks.co.uk/readme/video_timing.htm
    - http://martin.hinner.info/vga/pal.html
    - http://www.zxdesign.info/horiztiming.shtml

    ## zlib/libpng license

    Copyright (c) 2018 Andre Weissflog
    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.
    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:
        1. The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software in a
        product, an acknowledgment in the product documentation would be
        appreciated but is not required.
        2. Altered source versions must be plainly marked as such, and must not
        be misrepresented as being the original software.
        3. This notice may not be removed or altered from any source
        distribution. 
#*/
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* video standards */
typedef enum {
    CRT_PAL,
    CRT_NTSC
} crt_std;

/* constants */
#define CRT_PAL_H_DISPLAY_START (6)
#define CRT_PAL_V_DISPLAY_START (5)
/* FIXME: hmm, this is not used currently, since hsync/vsync signal switch to blank */
#define CRT_PAL_V_DISPLAY_END   (6+304)
#define CRT_PAL_H_DISPLAY_END   (6+52)

/* CRT state */
typedef struct {
    /* configuration parameters */
    int video_standard;     /* CRT_PAL or CRT_NTSC */
    int vis_x0, vis_x1, vis_y0, vis_y1;     /* visible region */

    /* current public state */
    int pos_x, pos_y;       /* current beam position in visible region */
    bool visible;           /* true if beam is current in visible region */

    /* internal counters */
    int h_pos;      /* current horizontal position (0..63) */
    int v_pos;      /* current vertical position (0..312) */
    bool h_sync;    /* last state of hsync, to detect raising edge trigger */
    bool v_sync;    /* last state of vsync, to detect raising edge trigger */
    bool h_blank;   /* horizontal blanking active (retrace + porches) */
    bool v_blank;   /* vertical blanking active */
    int h_retrace;  /* horizontal retrace counter, this is >0 during horizontal retrace */
    int v_retrace;  /* vertical retrace counter, this is >0 during vertical retrace */
} crt_t;

/*
    crt_init

    Initialize a new crt_t instance with the video standard and visible
    area. A pixel in the coordinate system is 1 microsecond wide (which
    is also the rate at which the tick function must be called), and
    1 scanline in the vertical direction.

    crt         -- pointer to a crt_t instance
    video_std   -- the video standard (PAL or NTSC)
    vis_x       -- left border of the visible region
    vis_y       -- top border of the visible region
    vis_w       -- width of the visible region in '1us pixels'
    vis_h       -- height of the visible region in scanlines

    NOTE: currently only PAL is supported!
*/
extern void crt_init(crt_t* crt, crt_std video_std, int vis_x, int vis_y, int vis_w, int vis_h);

/*
    crt_reset

    This will reset all internal counters, but keep the configuration values.
*/
extern void crt_reset(crt_t* crt);

/*
    crt_tick

    Call the tick function each microsecond of emulator time, and provide
    the current state the HSYNC and VSYNC signals. After calling
    the tick function, inspect the pos_x, pos_y and visible flag
    to check if the beam is currently in the visible region, and get the
    beam position in the visible area. The return value will also be true
    if the beam is currently in the visible region.

    crt         -- pointer to a crt_t instance
    hsync       -- current state of the hsync signal
    vsync       -- current sttae of the vsync signal
*/
extern bool crt_tick(crt_t* crt, bool hsync, bool vsync);

#ifdef __cplusplus
} /* extern "C" */
#endif
