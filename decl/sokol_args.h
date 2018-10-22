#pragma once
/*
    sokol_args.h    -- cross-platform key/value arg-parsing for web and native

    Do this:
        #define SOKOL_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following defines with your own implementations:

    SOKOL_ASSERT(c)     - your own assert macro (default: assert(c))
    SOKOL_LOG(msg)      - your own logging functions (default: puts(msg))
    SOKOL_CALLOC(n,s)   - your own calloc() implementation (default: calloc(n,s))
    SOKOL_FREE(p)       - your own free() implementation (default: free(p))
    SOKOL_API_DECL      - public function declaration prefix (default: extern)
    SOKOL_API_IMPL      - public function implementation prefix (default: -)

    OVERVIEW
    ========
    sokol_args.h provides a simple unified argument parsing API for WebAssembly and
    native apps.

    When running as WebAssembly app, arguments are taken from the page URL:

    https://floooh.github.io/tiny8bit/kc85.html?type=kc85_3&mod=m022&snapshot=kc85/jungle.kcc

    The same arguments provided to a command line app:

    kc85 type=kc85_3 mod=m022 snapshot=kc85/jungle.kcc

    ARGUMENT FORMATTING
    ===================
    On the web platform, arguments must be formatted as a valid URL query string 
    with 'percent encoding' used for special characters.

    Strings are expected to be UTF-8 encoded (although sokol_args.h doesn't
    contain any special UTF-8 handling). See below on how to obtain
    UTF-8 encoded argc/argv values on Windows when using WinMain() as
    entry point.

    On native platforms the following rules must be followed:

    Arguments have the general form

        key=value

    Key/value pairs are separated by 'whitespace', valid whitespace
    characters are space and tab.
    
    Whitespace characters in front and after the separating '=' character
    are ignored:

        key = value

    ...is the same as

        key=value

    The 'key' string must be a simple string without escape sequences or whitespace.

    Currently 'single keys' without values are not allowed, but may be 
    in the future.

    The 'value' string can be quoted, and quoted value strings can contain
    whitespace:

        key = 'single-quoted value'
        key = "double-quoted value"

    Single-quoted value strings can contain double quotes, and vice-versa:

        key = 'single-quoted value "can contain double-quotes"'
        key = "double-quoted value 'can contain single-quotes'"

    Note that correct quoting can be tricky on some shells, since command
    shells may remove quotes, unless they're escaped.

    Value strings can contain a small selection of escape sequences:

        \n  - newline
        \r  - carriage return
        \t  - tab
        \\  - escaped backslash
    
    (more escape codes may be added in the future).

    CODE EXAMPLE
    ============

        int main(int argc, char* argv[]) {
            // initialize sokol_args with default parameters
            sargs_setup(&(sargs_desc){
                .argc = argc,
                .argv = argv
            });

            // check if a key exists...
            if (sargs_exists("bla")) {
                ...
            }

            // get value string for key, if not found, return empty string ""
            const char* val0 = sargs_value("bla");

            // get value string for key, or default string if key not found
            const char* val1 = sargs_value_def("bla", "default_value");

            // check if a key matches expected value
            if (sargs_equals("type", "kc85_4")) {
                ...
            }

            // check if a key's value is "true", "yes" or "on"
            if (sargs_boolean("joystick_enabled")) {
                ...
            }

            // iterate over keys and values
            for (int i = 0; i < sargs_num_args(); i++) {
                printf("key: %s, value: %s\n", sargs_key_at(i), sargs_value_at(i));
            }

            // lookup argument index by key string, will return -1 if key
            // is not found, sargs_key_at() and sargs_value_at() will return
            // an empty string for invalid indices
            int index = sargs_find("bla");
            printf("key: %s, value: %s\n", sargs_key_at(index), sargs_value_at(index));

            // shutdown sokol-args
            sargs_shutdown();
        }

    WINMAIN AND ARGC / ARGV
    =======================
    On Windows with WinMain() based apps, use the __argc and __argv global
    variables provided Windows. These are compatible with main(argc, argv)
    and have already been converted to UTF-8 by Windows:

        int WINAPI WinMain(...) {
            sargs_setup(&(sargs_desc){
                .argc = __argc,
                .argv = __argv
            });
        }

    (this is also what sokol_app.h uses btw)

    API DOCUMENTATION
    =================
    void sargs_setup(const sargs_desc* desc)
        Initialize sokol_args, desc contains the following configuration
        parameters:
            int argc        - the main function's argc parameter
            char** argv     - the main function's argv parameter
            int max_args    - max number of key/value pairs, default is 16
            int buf_size    - size of the internal string buffer, default is 16384
        
        Note that on the web, argc and argv will be ignored and the arguments
        will be taken from the page URL instead.

        sargs_setup() will allocate 2 memory chunks: one for keeping track
        of the key/value args of size 'max_args*8', and a string buffer
        of size 'buf_size'.

    void sargs_shutdown(void)
        Shutdown sokol-args and free any allocated memory.

    bool sargs_isvalid(void)
        Return true between sargs_setup() and sargs_shutdown()

    bool sargs_exists(const char* key)
        Test if a key arg exists.

    const char* sargs_value(const char* key)
        Return value associated with key. Returns an empty
        string ("") if the key doesn't exist.

    const char* sargs_value_def(const char* key, const char* default)
        Return value associated with key, or the provided default
        value if the value doesn't exist.

    bool sargs_equals(const char* key, const char* val);
        Return true if the value associated with key matches
        the 'val' argument.

    bool sargs_boolean(const char* key)
        Return true if the value string of 'key' is one
        of 'true', 'yes', 'on'.

    int sargs_find(const char* key)
        Find argument by key name and return its index, or -1 if not found.

    int sargs_num_args(void)
        Return number of key/value pairs.

    const char* sargs_key_at(int index)
        Return the key name of argument at index. Returns empty string if
        is index is outside range.

    const char* sargs_value_at(int index)
        Return the value of argument at index. Returns empty string
        if index is outside range.

    TODO
    ====
    - parsing errors?

    LICENSE
    =======

    zlib/libpng license

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

/* setup sokol-args */
SOKOL_API_DECL void sargs_setup(const sargs_desc* desc);
/* shutdown sokol-args */
SOKOL_API_DECL void sargs_shutdown(void);
/* true between sargs_setup() and sargs_shutdown() */
SOKOL_API_DECL bool sargs_isvalid(void);
/* test if an argument exists by key name */
SOKOL_API_DECL bool sargs_exists(const char* key);
/* get value by key name, return empty string if key doesn't exist */
SOKOL_API_DECL const char* sargs_value(const char* key);
/* get value by key name, return provided default if key doesn't exist */
SOKOL_API_DECL const char* sargs_value_def(const char* key, const char* def);
/* return true if val arg matches the value associated with key */
SOKOL_API_DECL bool sargs_equals(const char* key, const char* val);
/* return true if key's value is "true", "yes" or "on" */
SOKOL_API_DECL bool sargs_boolean(const char* key);
/* get index of arg by key name, return -1 if not exists */
SOKOL_API_DECL int sargs_find(const char* key);
/* get number of parsed arguments */
SOKOL_API_DECL int sargs_num_args(void);
/* get key name of argument at index, or empty string */
SOKOL_API_DECL const char* sargs_key_at(int index);
/* get value string of argument at index, or empty string */
SOKOL_API_DECL const char* sargs_value_at(int index);

#ifdef __cplusplus
} /* extern "C" */
#endif
