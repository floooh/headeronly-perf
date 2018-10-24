#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifndef SOKOL_API_DECL
    #define SOKOL_API_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sample_rate;        /* requested sample rate */
    int num_channels;       /* number of channels, default: 1 (mono) */
    int buffer_frames;      /* number of frames in streaming buffer */
    int packet_frames;      /* number of frames in a packet */
    int num_packets;        /* number of packets in packet queue */
    void (*stream_cb)(float* buffer, int num_frames, int num_channels);  /* optional streaming callback */
} saudio_desc;

SOKOL_API_DECL void saudio_setup(const saudio_desc* desc);
SOKOL_API_DECL void saudio_shutdown(void);
SOKOL_API_DECL bool saudio_isvalid(void);
SOKOL_API_DECL int saudio_sample_rate(void);
SOKOL_API_DECL int saudio_buffer_size(void);
SOKOL_API_DECL int saudio_channels(void);
SOKOL_API_DECL int saudio_expect(void);
SOKOL_API_DECL int saudio_push(const float* frames, int num_frames);

#ifdef __cplusplus
} /* extern "C" */
#endif
