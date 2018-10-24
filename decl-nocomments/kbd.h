#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KBD_MAX_COLUMNS (12)
#define KBD_MAX_LINES (12)
#define KBD_MAX_MOD_KEYS (4)
#define KBD_MAX_KEYS (256)
#define KBD_MAX_PRESSED_KEYS (4)

typedef struct {
    int key;
    uint32_t mask;
    uint32_t pressed_frame;
    uint32_t released_frame;
} key_state_t;

typedef struct {
    uint32_t frame_count;
    int sticky_count;
    uint16_t active_columns;
    uint16_t active_lines;
    uint32_t key_masks[KBD_MAX_KEYS];
    uint32_t mod_masks[KBD_MAX_MOD_KEYS];
    key_state_t key_buffer[KBD_MAX_PRESSED_KEYS];
} kbd_t;

extern void kbd_init(kbd_t* kbd, int sticky_count);
extern void kbd_update(kbd_t* kbd);
extern void kbd_register_modifier(kbd_t* kbd, int layer, int column, int line);
extern void kbd_register_modifier_line(kbd_t* kbd, int layer, int line);
extern void kbd_register_modifier_column(kbd_t* kbd, int layer, int column);
extern void kbd_register_key(kbd_t* kbd, int key, int column, int line, int mod_mask);
extern void kbd_key_down(kbd_t* kbd, int key);
extern void kbd_key_up(kbd_t* kbd, int key);
extern uint16_t kbd_test_lines(kbd_t* kbd, uint16_t column_mask);
extern uint16_t kbd_test_columns(kbd_t* kbd, uint16_t line_mask);
extern void kbd_set_active_columns(kbd_t* kbd, uint16_t column_mask);
extern uint16_t kbd_scan_lines(kbd_t* kbd);
extern void kbd_set_active_lines(kbd_t* kbd, uint16_t line_mask);
extern uint16_t kbd_scan_columns(kbd_t* kbd);

#ifdef __cplusplus
} /* extern "C" */
#endif
