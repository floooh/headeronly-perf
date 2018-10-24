#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define Z1013_DISPLAY_WIDTH (256)
#define Z1013_DISPLAY_HEIGHT (256)

typedef enum {
    Z1013_TYPE_64,      /* Z1013.64 (default, latest model with 2 MHz and 64 KB RAM, new ROM) */
    Z1013_TYPE_16,      /* Z1013.16 (2 MHz model with 16 KB RAM, new ROM) */
    Z1013_TYPE_01,      /* Z1013.01 (original model, 1 MHz, 16 KB RAM) */
} z1013_type_t;

typedef struct {
    z1013_type_t type;          /* default is Z1013_TYPE_64 */

    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 256*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    const void* rom_mon202;
    const void* rom_mon_a2;
    const void* rom_font;
    int rom_mon202_size;
    int rom_mon_a2_size;
    int rom_font_size;
} z1013_desc_t;

typedef struct {
    z80_t cpu;
    z80pio_t pio;
    bool valid;
    z1013_type_t type;
    uint8_t kbd_request_column;
    bool kbd_request_line_hilo;
    uint32_t* pixel_buffer;
    clk_t clk;
    mem_t mem;
    kbd_t kbd;
    uint8_t ram[1<<16];
    uint8_t rom_os[2048];
    uint8_t rom_font[2048];
} z1013_t;

extern void z1013_init(z1013_t* sys, const z1013_desc_t* desc);
extern void z1013_discard(z1013_t* sys);
extern void z1013_reset(z1013_t* sys);
extern void z1013_exec(z1013_t* sys, uint32_t micro_seconds);
extern void z1013_key_down(z1013_t* sys, int key_code);
extern void z1013_key_up(z1013_t* sys, int key_code);
extern bool z1013_quickload(z1013_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
