#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define C64_DISPLAY_WIDTH (392)             /* required framebuffer width in pixels */
#define C64_DISPLAY_HEIGHT (272)            /* required framebuffer hight in pixels */
#define C64_MAX_AUDIO_SAMPLES (1024)        /* max number of audio samples in internal sample buffer */
#define C64_DEFAULT_AUDIO_SAMPLES (128)     /* default number of samples in internal sample buffer */ 
#define C64_MAX_TAPE_SIZE (512*1024)        /* max size of cassette tape image */

typedef enum {
    C64_JOYSTICKTYPE_NONE,
    C64_JOYSTICKTYPE_DIGITAL_1,
    C64_JOYSTICKTYPE_DIGITAL_2,
    C64_JOYSTICKTYPE_PADDLE_1,  /* FIXME: not emulated */
    C64_JOYSTICKTYPE_PADDLE_2,  /* FIXME: not emulated */
} c64_joystick_type_t;

#define C64_JOYSTICK_UP    (1<<0)
#define C64_JOYSTICK_DOWN  (1<<1)
#define C64_JOYSTICK_LEFT  (1<<2)
#define C64_JOYSTICK_RIGHT (1<<3)
#define C64_JOYSTICK_BTN   (1<<4)

typedef void (*c64_audio_callback_t)(const float* samples, int num_samples, void* user_data);

typedef struct {
    c64_joystick_type_t joystick_type;  /* default is C64_JOYSTICK_NONE */

    void* pixel_buffer;         /* pointer to a linear RGBA8 pixel buffer, at least 392*272*4 bytes */
    int pixel_buffer_size;      /* size of the pixel buffer in bytes */

    void* user_data;

    c64_audio_callback_t audio_cb;  /* called when audio_num_samples are ready */
    int audio_num_samples;          /* default is C64_AUDIO_NUM_SAMPLES */
    int audio_sample_rate;          /* playback sample rate, default is 44100 */
    float audio_sid_volume;         /* audio volume of the SID chip (0.0 .. 1.0), default is 1.0 */
    float audio_beeper_volume;      /* audio volume of the tape-beeper (0.0 .. 1.0), default is 0.1 */
    bool audio_tape_sound;          /* if true, tape loading is audible */

    const void* rom_char;           /* 4 KByte character ROM dump */
    const void* rom_basic;          /* 8 KByte BASIC dump */
    const void* rom_kernal;         /* 8 KByte KERNAL dump */
    int rom_char_size;
    int rom_basic_size;
    int rom_kernal_size;
} c64_desc_t;

typedef struct {
    m6502_t cpu;
    m6526_t cia_1;
    m6526_t cia_2;
    m6569_t vic;
    m6581_t sid;
    beeper_t beeper;            /* used for the tape-sound */
    
    bool valid;
    c64_joystick_type_t joystick_type;
    uint8_t joystick_active;
    bool io_mapped;             /* true when D000..DFFF is has IO area mapped in */
    uint8_t cpu_port;           /* last state of CPU port (for memory mapping) */
    uint8_t kbd_joy1_mask;      /* current joystick-1 state from keyboard-joystick emulation */
    uint8_t kbd_joy2_mask;      /* current joystick-2 state from keyboard-joystick emulation */
    uint8_t joy_joy1_mask;      /* current joystick-1 state from c64_joystick() */
    uint8_t joy_joy2_mask;      /* current joystick-2 state from c64_joystick() */
    uint16_t vic_bank_select;   /* upper 4 address bits from CIA-2 port A */

    clk_t clk;
    kbd_t kbd;
    mem_t mem_cpu;
    mem_t mem_vic;

    void* user_data;
    uint32_t* pixel_buffer;
    c64_audio_callback_t audio_cb;
    int num_samples;
    int sample_pos;
    float sample_buffer[C64_MAX_AUDIO_SAMPLES];

    uint8_t color_ram[1024];        /* special static color ram */
    uint8_t ram[1<<16];             /* general ram */
    uint8_t rom_char[0x1000];       /* 4 KB character ROM image */
    uint8_t rom_basic[0x2000];      /* 8 KB BASIC ROM image */
    uint8_t rom_kernal[0x2000];     /* 8 KB KERNAL V3 ROM image */

    bool tape_motor;    /* tape motor on/off */
    bool tape_button;   /* play button on tape pressed/unpressed */
    bool tape_sound;    /* true when tape is audible */
    int tape_size;      /* tape_size > 0: a tape is inserted */
    int tape_pos;        
    int tape_tick_count;
    uint8_t tape_buf[C64_MAX_TAPE_SIZE];
} c64_t;

extern void c64_init(c64_t* sys, const c64_desc_t* desc);
extern void c64_discard(c64_t* sys);
extern void c64_reset(c64_t* sys);
extern void c64_exec(c64_t* sys, uint32_t micro_seconds);
extern void c64_key_down(c64_t* sys, int key_code);
extern void c64_key_up(c64_t* sys, int key_code);
extern void c64_set_joystick_type(c64_t* sys, c64_joystick_type_t type);
extern c64_joystick_type_t c64_joystick_type(c64_t* sys);
extern void c64_joystick(c64_t* sys, uint8_t joy1_mask, uint8_t joy2_mask);
extern bool c64_insert_tape(c64_t* sys, const uint8_t* ptr, int num_bytes);
extern void c64_remove_tape(c64_t* sys);
extern void c64_start_tape(c64_t* sys);
extern void c64_stop_tape(c64_t* sys);
extern bool c64_quickload(c64_t* sys, const uint8_t* ptr, int num_bytes);

#ifdef __cplusplus
} /* extern "C" */
#endif
