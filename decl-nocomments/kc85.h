#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define KC85_DISPLAY_WIDTH (320)
#define KC85_DISPLAY_HEIGHT (256)
#define KC85_MAX_AUDIO_SAMPLES (1024)       /* max number of audio samples in internal sample buffer */
#define KC85_DEFAULT_AUDIO_SAMPLES (128)    /* default number of samples in internal sample buffer */ 
#define KC85_MAX_TAPE_SIZE (64 * 1024)      /* max size of a snapshot file in bytes */
#define KC85_NUM_SLOTS (2)                  /* 2 expansion slots in main unit, each needs one mem_t layer! */
#define KC85_EXP_BUFSIZE (KC85_NUM_SLOTS*64*1024) /* expansion system buffer size (64 KB per slot) */

#define KC85_PIO_A_CAOS_ROM        (1<<0)
#define KC85_PIO_A_RAM             (1<<1)
#define KC85_PIO_A_IRM             (1<<2)
#define KC85_PIO_A_RAM_RO          (1<<3)
#define KC85_PIO_A_UNUSED          (1<<4)
#define KC85_PIO_A_TAPE_LED        (1<<5)
#define KC85_PIO_A_TAPE_MOTOR      (1<<6)
#define KC85_PIO_A_BASIC_ROM       (1<<7)
#define KC85_PIO_B_VOLUME_MASK     ((1<<5)-1)
#define KC85_PIO_B_RAM8            (1<<5)  /* KC85/4 only */
#define KC85_PIO_B_RAM8_RO         (1<<6)  /* KC85/4 only */
#define KC85_PIO_B_BLINK_ENABLED   (1<<7)
#define KC85_IO84_SEL_VIEW_IMG     (1<<0)  /* 0: display img0, 1: display img1 */
#define KC85_IO84_SEL_CPU_COLOR    (1<<1)  /* 0: access pixels, 1: access colors */
#define KC85_IO84_SEL_CPU_IMG      (1<<2)  /* 0: access img0, 1: access img1 */
#define KC85_IO84_HICOLOR          (1<<3)  /* 0: hicolor mode off, 1: hicolor mode on */
#define KC85_IO84_SEL_RAM8         (1<<4)  /* select RAM8 block 0 or 1 */
#define KC85_IO84_BLOCKSEL_RAM8    (1<<5)  /* no idea what that does...? */
#define KC85_IO86_RAM4             (1<<0)
#define KC85_IO86_RAM4_RO          (1<<1)
#define KC85_IO86_CAOS_ROM_C       (1<<7)

typedef enum {
    KC85_TYPE_2,        /* KC85/2 (default) */
    KC85_TYPE_3,        /* KC85/3 */
    KC85_TYPE_4,        /* KC85/4 */
} kc85_type_t;

typedef void (*kc85_audio_callback_t)(const float* samples, int num_samples, void* user_data);
typedef void (*kc85_patch_callback_t)(const char* snapshot_name, void* user_data);

typedef struct {
    kc85_type_t type;           /* default is KC85_TYPE_2 */
    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 320*256*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */
    void* user_data;
    kc85_audio_callback_t audio_cb;     /* called when audio_num_samples are ready */
    int audio_num_samples;              /* default is KC85_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;              /* playback sample rate, default is 44100 */
    float audio_volume;                 /* audio volume (0.0 .. 1.0), default is 0.4 */
    kc85_patch_callback_t patch_cb;
    const void* rom_caos22;             /* CAOS 2.2 (used in KC85/2) */
    const void* rom_caos31;             /* CAOS 3.1 (used in KC85/3) */
    const void* rom_caos42c;            /* CAOS 4.2 at 0xC000 (KC85/4) */
    const void* rom_caos42e;            /* CAOS 4.2 at 0xE000 (KC85/4) */
    const void* rom_kcbasic;            /* same BASIC version for KC85/3 and KC85/4 */
    int rom_caos22_size;
    int rom_caos31_size;
    int rom_caos42c_size;
    int rom_caos42e_size;
    int rom_kcbasic_size;
} kc85_desc_t;

typedef enum {
    KC85_MODULE_NONE,
    KC85_MODULE_M006_BASIC,         /* BASIC+CAOS 16K ROM module for the KC85/2 (id=0xFC) */
    KC85_MODULE_M011_64KBYE,        /* 64 KByte RAM expansion (id=0xF6) */
    KC85_MODULE_M012_TEXOR,         /* TEXOR text editing (id=0xFB) */
    KC85_MODULE_M022_16KBYTE,       /* 16 KByte RAM expansion (id=0xF4) */
    KC85_MODULE_M026_FORTH,         /* FORTH IDE (id=0xFB) */
    KC85_MODULE_M027_DEVELOPMENT,   /* Assembler IDE (id=0xFB) */

    KC85_MODULE_NUM,
} kc85_module_type_t;

typedef struct {
    kc85_module_type_t type;
    uint8_t id;                     /* id of currently inserted module */
    bool writable;                  /* RAM or ROM module */
    uint8_t addr_mask;              /* the module's address mask */
    int size;                       /* the module's byte size */
} kc85_module_t;

typedef struct {
    uint8_t addr;                   /* 0x0C (left slot) or 0x08 (right slot) */
    uint8_t ctrl;                   /* current control byte */
    uint32_t buf_offset;            /* byte-offset in expansion system data buffer */
    kc85_module_t mod;              /* attributes of currently inserted module */
} kc85_slot_t;

typedef struct {
    kc85_slot_t slot[KC85_NUM_SLOTS];   /* slots 0x08 and 0x0C in KC85 main unit */
    uint32_t buf_top;                   /* offset of free area in expansion buffer (kc85_t.exp_buf[]) */
} kc85_exp_t;

typedef struct {
    z80_t cpu;
    z80ctc_t ctc;
    z80pio_t pio;
    beeper_t beeper_1;
    beeper_t beeper_2;

    bool valid;
    kc85_type_t type;
    uint8_t pio_a;          /* current PIO-A value, used for bankswitching */
    uint8_t pio_b;          /* current PIO-B value, used for bankswitching */
    uint8_t io84;           /* byte latch at port 0x84, only on KC85/4 */
    uint8_t io86;           /* byte latch at port 0x86, only on KC85/4 */
    bool blink_flag;        /* foreground color blinking flag toggled by CTC */

    int scanline_period;
    int scanline_counter;
    int cur_scanline;

    clk_t clk;
    kbd_t kbd;
    mem_t mem;
    kc85_exp_t exp;         /* expansion module system */

    uint32_t* pixel_buffer;
    void* user_data;
    kc85_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[KC85_MAX_AUDIO_SAMPLES];
    kc85_patch_callback_t patch_cb;

    uint8_t ram[8][0x4000];             /* up to 8 16-KByte RAM banks */
    uint8_t rom_basic[0x2000];          /* 8 KByte BASIC ROM (KC85/3 and /4 only) */
    uint8_t rom_caos_c[0x1000];         /* 4 KByte CAOS ROM at 0xC000 (KC85/4 only) */
    uint8_t rom_caos_e[0x2000];         /* 8 KByte CAOS ROM at 0xE000 */
    uint8_t exp_buf[KC85_EXP_BUFSIZE];  /* expansion system RAM/ROM */
} kc85_t;

void kc85_init(kc85_t* sys, const kc85_desc_t* desc);
void kc85_discard(kc85_t* sys);
void kc85_reset(kc85_t* sys);
void kc85_exec(kc85_t* sys, uint32_t micro_seconds);
void kc85_key_down(kc85_t* sys, int key_code);
void kc85_key_up(kc85_t* sys, int key_code);
bool kc85_insert_ram_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type);
bool kc85_insert_rom_module(kc85_t* sys, uint8_t slot, kc85_module_type_t type, const void* rom_ptr, int rom_size);
bool kc85_remove_module(kc85_t* sys, uint8_t slot);
kc85_slot_t* kc85_slot_by_addr(kc85_t* sys, uint8_t slot_addr);
bool kc85_slot_occupied(kc85_t* sys, uint8_t slot_addr);
bool kc85_slot_cpu_visible(kc85_t* sys, uint8_t slot_addr);
uint16_t kc85_slot_cpu_addr(kc85_t* sys, uint8_t slot_addr);
bool kc85_quickload(kc85_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
