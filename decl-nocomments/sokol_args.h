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
    int argc;
    char** argv;
    int max_args;
    int buf_size;
} sargs_desc;

SOKOL_API_DECL void sargs_setup(const sargs_desc* desc);
SOKOL_API_DECL void sargs_shutdown(void);
SOKOL_API_DECL bool sargs_isvalid(void);
SOKOL_API_DECL bool sargs_exists(const char* key);
SOKOL_API_DECL const char* sargs_value(const char* key);
SOKOL_API_DECL const char* sargs_value_def(const char* key, const char* def);
SOKOL_API_DECL bool sargs_equals(const char* key, const char* val);
SOKOL_API_DECL bool sargs_boolean(const char* key);
SOKOL_API_DECL int sargs_find(const char* key);
SOKOL_API_DECL int sargs_num_args(void);
SOKOL_API_DECL const char* sargs_key_at(int index);
SOKOL_API_DECL const char* sargs_value_at(int index);

#ifdef __cplusplus
} /* extern "C" */
#endif
