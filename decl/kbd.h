#pragma once
/*
    kbd.h -- keyboard matrix helpers

    Do this:
        #define CHIPS_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following macros with your own implementation
    
        CHIPS_ASSERT(c)     -- your own assert macro (default: assert(c))

    OVERVIEW

    A kbd_t instance maps key codes to the cross-sections of
    an up to 12x12 keyboard matrix with up to 4 modifier keys (shift, ctrl, ...)

        C0  C1  C1  C2  C3 ... C12
         /   /   /   /   /       |
    L0---+---+---+---+---+-....--+
         |   /   /   /   /       /
    L1---+---+---+---+---+-....--+
         .   .   .   .   .       .
    L12--+---+---+---+---+-....--+

    First register host-system key codes with the keyboard matrix (where are keys
    positioned in the matrix) by calling kbd_register_modifier() and
    kbd_register_key().

    Feed 'host system key presses' into the keyboard_matrix instance
    by calling kbd_key_down() and kbd_key_up(). Some emulated systems
    took quite long to scan a key press, so the keyboard matrix has
    a 'sticky count', which may prolong the key press visible to the
    emulated system if the host-system key press was too short.

    Call the function kbd_test_lines() to check the current state of the
    keyboard matrix.

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
*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_COLUMNS (12)
#define KBD_MAX_LINES (12)
#define KBD_MAX_MOD_KEYS (4)
#define KBD_MAX_KEYS (256)
#define KBD_MAX_PRESSED_KEYS (4)

/* a pressed-key state */
typedef struct {
    /* key code of the pressed key */
    int key;
    /* mask bit layout is 8-bits modifier, and 12-bits each columns and lines */
    /* |SSSSSSSS|CCCCCCCCCCCC|LLLLLLLLLLLL| */
    uint32_t mask;
    /* the frame-count when the key was pressed down */
    uint32_t pressed_frame;
    /* the frame-count when the key was released, 0 if not yet released */
    uint32_t released_frame;
} key_state_t;

/* keyboard matrix state */
typedef struct {
    /* current frame counter, bumped by kbd_update() */
    uint32_t frame_count;
    /* number of frames a key will at least remain pressed */
    int sticky_count;
    /* currently active columns */
    uint16_t active_columns;
    /* currently active lines */
    uint16_t active_lines;
    /* map key ASCII code to modifier/column/line bits */
    uint32_t key_masks[KBD_MAX_KEYS];
    /* column/line bits for modifier keys */
    uint32_t mod_masks[KBD_MAX_MOD_KEYS];
    /* currently pressed keys (bitmask==0 is empty slot) */
    key_state_t key_buffer[KBD_MAX_PRESSED_KEYS];
} kbd_t;

/* initialize a keyboard matrix instance */
extern void kbd_init(kbd_t* kbd, int sticky_count);
/* update keyboard matrix state (releases sticky keys), usually call once per frame */
extern void kbd_update(kbd_t* kbd);
/* register a modifier key, layers are between from 0 to KBD_MAX_MOD_KEYS-1 */
extern void kbd_register_modifier(kbd_t* kbd, int layer, int column, int line);
/* register a modifier key where the modifier is mapped to an entire keyboard line */
extern void kbd_register_modifier_line(kbd_t* kbd, int layer, int line);
/* register a modifier key where the modifier is mapped to an entire keyboard column */
extern void kbd_register_modifier_column(kbd_t* kbd, int layer, int column);
/* register a key */
extern void kbd_register_key(kbd_t* kbd, int key, int column, int line, int mod_mask);
/* add a key to the pressed-key buffer */
extern void kbd_key_down(kbd_t* kbd, int key);
/* remove a key from the pressed-key buffer */
extern void kbd_key_up(kbd_t* kbd, int key);
/* test keyboard matrix against a column bitmask and return lit lines */
extern uint16_t kbd_test_lines(kbd_t* kbd, uint16_t column_mask);
/* test keyboard matrix against a line bitmask and return lit columns */
extern uint16_t kbd_test_columns(kbd_t* kbd, uint16_t line_mask);
/* set active column mask (use together with kbd_scan_lines */
extern void kbd_set_active_columns(kbd_t* kbd, uint16_t column_mask);
/* scan active lines (used together with kbd_set_active_columns */
extern uint16_t kbd_scan_lines(kbd_t* kbd);
/* set active lines mask (use together with kbd_scan_columns */
extern void kbd_set_active_lines(kbd_t* kbd, uint16_t line_mask);
/* scan active columns (used together with kbd_set_active_lines */
extern uint16_t kbd_scan_columns(kbd_t* kbd);

#ifdef __cplusplus
} /* extern "C" */
#endif
