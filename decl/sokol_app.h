#pragma once
/*
    sokol_app.h -- cross-platform application wrapper

    Do this:
        #define SOKOL_IMPL
    before you include this file in *one* C or C++ file to create the 
    implementation.

    Optionally provide the following defines with your own implementations:

    SOKOL_ASSERT(c)     - your own assert macro (default: assert(c))
    SOKOL_LOG(msg)      - your own logging function (default: puts(msg))
    SOKOL_UNREACHABLE() - a guard macro for unreachable code (default: assert(false))
    SOKOL_ABORT()       - called after an unrecoverable error (default: abort())
    SOKOL_WIN32_FORCE_MAIN  - define this on Win32 to use a main() entry point instead of WinMain
    SOKOL_API_DECL      - public function declaration prefix (default: extern)
    SOKOL_API_IMPL      - public function implementation prefix (default: -)

    Optionally define the following to force debug checks and validations
    even in release mode:

    SOKOL_DEBUG         - by default this is defined if _DEBUG is defined

    Portions of the Windows and Linux GL initialization and event code have been
    taken from GLFW (http://www.glfw.org/)

    iOS onscreen keyboard support 'inspired' by libgdx.

    If you use sokol_app.h together with sokol_gfx.h, include both headers
    in the implementation source file, and include sokol_app.h before
    sokol_gfx.h since sokol_app.h will also include the required 3D-API 
    headers.

    On Windows, a minimal 'GL header' and function loader is integrated which
    contains just enough of GL for sokol_gfx.h. If you want to use your own
    GL header-generator/loader instead, define SOKOL_WIN32_NO_GL_LOADER
    before including the implementation part of sokol_app.h.

    TEMP NOTE DUMP
    ==============
    - need a way to quit application programmatically (sapp_request_quit())
    - need a way to intercept a pending quit via UI close button (could be
      done via frame_cb return value, and a sapp_quit_requested() function)
    - onscreen keyboard support on Android requires Java :(, should we even bother?
    - sapp_desc needs a bool whether to initialize depth-stencil surface
    - GL context initialization needs more control (at least what GL version to initialize)
    - application icon
    - mouse pointer visibility(?)
    - the UPDATE_CURSOR event currently behaves differently between Win32 and OSX
      (Win32 sends the event each frame when the mouse moves and is inside the window
      client area, OSX sends it only once when the mouse enters the client area)

    FIXME: ERROR HANDLING (this will need an error callback function)

    FEATURE OVERVIEW
    ================
    sokol_app.h provides a minimalistic cross-platform API which
    implements the 'application-wrapper' parts of a 3D application:

    - a common application entry function
    - creates a window and 3D-API context/device with a 'default framebuffer'
    - makes the rendered frame visible
    - provides keyboard-, mouse- and low-level touch-events
    - platforms: MacOS, iOS, HTML5, Win32, Linux (planned: Android, RaspberryPi)
    - 3D-APIs: Metal, D3D11, GL3.2, GLES2, GLES3, WebGL, WebGL2

    FEATURE/PLATFORM MATRIX
    =======================
                        | Windows | macOS | Linux |  iOS  | Android | Raspi | HTML5
    --------------------+---------+-------+-------+-------+---------+-------+-------
    gl 3.x              | YES     | ---   | YES   | ---   | ---     | ---   | ---
    gles2/webgl         | ---     | ---   | ---   | YES   | TODO    | TODO  | YES
    gles3/webgl2        | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    metal               | ---     | YES   | ---   | YES   | ---     | ---   | ---
    d3d11               | YES     | ---   | ---   | ---   | ---     | ---   | ---
    KEY_DOWN            | YES     | YES   | YES   | SOME  | TODO    | TODO  | YES
    KEY_UP              | YES     | YES   | YES   | SOME  | TODO    | TODO  | YES
    CHAR                | YES     | YES   | YES   | YES   | TODO    | TODO  | YES
    MOUSE_DOWN          | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    MOUSE_UP            | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    MOUSE_SCROLL        | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    MOUSE_MOVE          | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    MOUSE_ENTER         | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    MOUSE_LEAVE         | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    TOUCHES_BEGAN       | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    TOUCHES_MOVED       | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    TOUCHES_ENDED       | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    TOUCHES_CANCELLED   | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    RESIZED             | YES     | YES   | YES   | YES   | TODO    | ---   | YES
    ICONIFIED           | YES     | YES   | YES   | ---   | ---     | ---   | ---
    RESTORED            | YES     | YES   | YES   | ---   | ---     | ---   | ---
    SUSPENDED           | ---     | ---   | ---   | YES   | TODO    | ---   | TODO
    RESUMED             | ---     | ---   | ---   | YES   | TODO    | ---   | TODO
    UPDATE_CURSOR       | YES     | YES   | TODO  | ---   | ---     | ---   | TODO
    IME                 | TODO    | TODO? | TODO  | ???   | TODO    | ???   | ???
    windowed            | YES     | YES   | YES   | ---   | ---     | TODO  | YES
    fullscreen          | YES     | YES   | TODO  | YES   | TODO    | TODO  | ---
    pointer lock        | TODO    | TODO  | TODO  | ---   | ---     | TODO  | TODO
    screen keyboard     | ---     | ---   | ---   | YES   | TODO    | ---   | YES
    swap interval       | YES     | YES   | YES   | YES   | TODO    | TODO  | YES
    high-dpi            | YES     | YES   | TODO  | YES   | TODO    | TODO  | YES

    - what about bluetooth keyboard / mouse on mobile platforms?

    STEP BY STEP
    ============
    --- Add a sokol_main() to your code which returns a sapp_desc structure
        with initialization parameters and callback function pointers. This
        function is called very early, usually at the start of the 
        platform's entry function (e.g. main or WinMain). You should do as
        little as possible here, since the rest of your code might be called
        from another thread (this depends on the platform):

            sapp_desc sokol_main(int argc, char* argv[]) {
                return (sapp_desc) {
                    .width = 640,
                    .height = 480,
                    .init_cb = my_init_func,
                    .frame_cb = my_frame_func,
                    .cleanup_cb = my_cleanup_func,
                    .event_cb = my_event_func,
                    ...
                };
            }

        There are many more setup parameters, but these are the most important.
        For a complete list search for the sapp_desc structure declaration
        below.

        DO NOT call any sokol-app function from inside sokol_main(), since
        sokol-app will not be initialized at this point. 

        The .width and .height parameters are the preferred size of the 3D
        rendering canvas. The actual size may differ from this depending on
        platform and other circumstances. Also the canvas size may change at
        any time (for instance when the user resizes the application window,
        or rotates the mobile device).

        All provided function callbacks will be called from the same thread,
        but this may be different from the thread where sokol_main() was called.

        .init_cb (void (init_cb*)(void))
            This function is called once after the application window,
            3D rendering context and swap chain have been created. The
            function takes no arguments and has no return value.
        .frame_cb (void (frame_cb*)(void))
            This is the per-frame callback, which is usually called 60
            times per second. This is where your application would update
            most of its state and perform all rendering.
        .cleanup_cb (void (cleanup_cb*)(void))
            The cleanup callback is called once right before the application
            quits.
        .event_cb (void (event-cb*)(const sapp_event* event))
            The event callback is mainly for input handling, but in the
            future may also be used to communicate other types of events
            to the application. Keep the event_cb struct member zero-initialized
            if your application doesn't require event handling.
                      
    --- Implement the initialization callback function, this is called once
        after the rendering surface, 3D API and swap chain have been
        initialized by sokol_app. All sokol-app functions can be called
        from inside the initialization callback, the most useful functions
        at this point are:

        int sapp_width(void)
            Returns the current width of the default framebuffer, this may change
            from one frame to the next.
        int sapp_height(void)
            Likewise, returns the current height of the default framebuffer.
        
        bool sapp_gles2(void)
            Returns true if as GLES2 or WebGL2 context had been created (for
            instance because GLES3/WebGL2 isn't available on the device)

        const void* sapp_metal_get_device(void)
        const void* sapp_metal_get_renderpass_descriptor(void)
        const void* sapp_metal_get_drawable(void)
            If the Metal backend has been selected, these functions return pointers
            to various Metal API objects required for rendering, otherwise
            they return a null pointer. Note that the returned pointers
            to the renderpass-descriptor and drawable may change from one
            frame to the next!

        const void* sapp_macos_get_window(void)
            On macOS, get the NSWindow object pointer, otherwise a null pointer.

        const void* sapp_ios_get_window(void)
            On iOS, get the UIWindow object pointer, otherwise a null pointer.

        const void* sapp_win32_get_hwnd(void)
            On Windows, get the window's HWND, otherwise a null pointer.

        const void* sapp_d3d11_get_device(void);
        const void* sapp_d3d11_get_device_context(void);
        const void* sapp_d3d11_get_render_target_view(void);
        const void* sapp_d3d11_get_depth_stencil_view(void);
            Similar to the sapp_metal_* functions, the sapp_d3d11_* functions
            return pointers to D3D11 API objects required for rendering, 
            only if the D3D11 backend has been selected. Otherwise they
            return a null pointer. Note that the returned pointers to the
            render-target-view and depth-stencil-view may change from one
            frame to the next!

    --- Implement the frame-callback function, this function will be called
        on the same thread as the init callback, but might be on a different
        thread than the sokol_main() function. Note that the size of
        the rendering framebuffer might have change since the frame callback
        was called last. Call the functions sapp_width() and sapp_height()
        to get the current size.

    --- Optionally implement the event-callback to handle input events.
        sokol-app provides the following type of input events:
            - a 'virtual key' was pressed down or released
            - a single text character was entered (provided as UTF-32 code point)
            - a mouse button was pressed down or released (left, right, middle)
            - mouse-wheel or 2D scrolling events
            - the mouse was moved
            - the mouse has entered or left the application window boundaries
            - low-level, portable multi-touch events (began, moved, ended, cancelled)
        More types of events will be added in the future (like window
        minimized, maximized, application life cycle events, etc...)

    --- Implement the cleanup-callback function, this is called once
        after the user quits the application (currently there's now way
        to quite the application programmatically)


    HIGH-DPI RENDERING
    ==================
    You can set the sapp_desc.high_dpi flag during initialization to request
    a full-resolution framebuffer on HighDPI displays. The default behaviour
    is sapp_desc.high_dpi=false, this means that the application will
    render to a lower-resolution framebuffer on HighDPI displays and the
    rendered content will be upscaled by the window system composer.

    In a HighDPI scenario, you still request the same window size during
    sokol_main(), but the framebuffer sizes returned by sapp_width()
    and sapp_height() will be scaled up according to the DPI scaling
    ratio. You can also get a DPI scaling factor with the function
    sapp_dpi_scale().

    Here's an example on a Mac with Retina display:

    sapp_desc sokol_main() {
        return (sapp_desc) {
            .width = 640,
            .height = 480,
            .high_dpi = true,
            ...
        };
    }

    The functions sapp_width(), sapp_height() and sapp_dpi_scale() will
    return the following values:

    sapp_width      -> 1280
    sapp_height     -> 960
    sapp_dpi_scale  -> 2.0

    If the high_dpi flag is false, or you're not running on a Retina display,
    the values would be:

    sapp_width      -> 640
    sapp_height     -> 480
    sapp_dpi_scale  -> 1.0

    FULLSCREEN
    ==========
    If the sapp_desc.fullscreen flag is true, sokol-app will try to create
    a fullscreen window on platforms with a 'proper' window system 
    (mobile devices will always use fullscreen). The implementation details
    depend on the target platform, in general sokol-app will use a
    'soft approach' which doesn't interfere too much with the platform's
    window system (for instance borderless fullscreen window instead of
    a 'real' fullscreen mode). Such details might change over time
    as sokol-app is adapted for different needs.

    The most important effect of fullscreen mode to keep in mind is that
    the requested canvas width and height will be ignored for the initial
    window size, calling sapp_width() and sapp_height() will instead return
    the resolution of the fullscreen canvas (however the provided size
    might still be used for the non-fullscreen window, in case the user can
    switch back from fullscreen- to windowed-mode).

    ONSCREEN KEYBOARD
    =================
    On some platforms which don't provide a physical keyboard, sokol-app
    can display the platform's integrated onscreen keyboard for text
    input. To request that the onscreen keyboard is shown, call

        sapp_show_keyboard(true);

    Likewise, to hide the keyboard call:

        sapp_show_keyboard(false);

    Note that on the web platform, the keyboard can only be shown from
    inside an input handler. On such platforms, sapp_show_keyboard()
    will only work as expected when it is called from inside the
    sokol-app event callback function. When called from other places,
    an internal flag will be set, and the onscreen keyboard will be
    called at the next 'legal' opportunity (when the next input event
    is handled).

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

enum {
    SAPP_MAX_TOUCHPOINTS = 8,
    SAPP_MAX_MOUSEBUTTONS = 3,
    SAPP_MAX_KEYCODES = 512,
};

typedef enum {
    SAPP_EVENTTYPE_INVALID,
    SAPP_EVENTTYPE_KEY_DOWN,
    SAPP_EVENTTYPE_KEY_UP,
    SAPP_EVENTTYPE_CHAR,
    SAPP_EVENTTYPE_MOUSE_DOWN,
    SAPP_EVENTTYPE_MOUSE_UP,
    SAPP_EVENTTYPE_MOUSE_SCROLL,
    SAPP_EVENTTYPE_MOUSE_MOVE,
    SAPP_EVENTTYPE_MOUSE_ENTER,
    SAPP_EVENTTYPE_MOUSE_LEAVE,
    SAPP_EVENTTYPE_TOUCHES_BEGAN,
    SAPP_EVENTTYPE_TOUCHES_MOVED,
    SAPP_EVENTTYPE_TOUCHES_ENDED,
    SAPP_EVENTTYPE_TOUCHES_CANCELLED,
    SAPP_EVENTTYPE_RESIZED,
    SAPP_EVENTTYPE_ICONIFIED,
    SAPP_EVENTTYPE_RESTORED,
    SAPP_EVENTTYPE_SUSPENDED,
    SAPP_EVENTTYPE_RESUMED,
    SAPP_EVENTTYPE_UPDATE_CURSOR,
    _SAPP_EVENTTYPE_NUM,
    _SAPP_EVENTTYPE_FORCE_U32 = 0x7FFFFFF
} sapp_event_type;

/* key codes are the same names and values as GLFW */
typedef enum {
    SAPP_KEYCODE_INVALID          = 0,
    SAPP_KEYCODE_SPACE            = 32,
    SAPP_KEYCODE_APOSTROPHE       = 39,  /* ' */
    SAPP_KEYCODE_COMMA            = 44,  /* , */
    SAPP_KEYCODE_MINUS            = 45,  /* - */
    SAPP_KEYCODE_PERIOD           = 46,  /* . */
    SAPP_KEYCODE_SLASH            = 47,  /* / */
    SAPP_KEYCODE_0                = 48,
    SAPP_KEYCODE_1                = 49,
    SAPP_KEYCODE_2                = 50,
    SAPP_KEYCODE_3                = 51,
    SAPP_KEYCODE_4                = 52,
    SAPP_KEYCODE_5                = 53,
    SAPP_KEYCODE_6                = 54,
    SAPP_KEYCODE_7                = 55,
    SAPP_KEYCODE_8                = 56,
    SAPP_KEYCODE_9                = 57,
    SAPP_KEYCODE_SEMICOLON        = 59,  /* ; */
    SAPP_KEYCODE_EQUAL            = 61,  /* = */
    SAPP_KEYCODE_A                = 65,
    SAPP_KEYCODE_B                = 66,
    SAPP_KEYCODE_C                = 67,
    SAPP_KEYCODE_D                = 68,
    SAPP_KEYCODE_E                = 69,
    SAPP_KEYCODE_F                = 70,
    SAPP_KEYCODE_G                = 71,
    SAPP_KEYCODE_H                = 72,
    SAPP_KEYCODE_I                = 73,
    SAPP_KEYCODE_J                = 74,
    SAPP_KEYCODE_K                = 75,
    SAPP_KEYCODE_L                = 76,
    SAPP_KEYCODE_M                = 77,
    SAPP_KEYCODE_N                = 78,
    SAPP_KEYCODE_O                = 79,
    SAPP_KEYCODE_P                = 80,
    SAPP_KEYCODE_Q                = 81,
    SAPP_KEYCODE_R                = 82,
    SAPP_KEYCODE_S                = 83,
    SAPP_KEYCODE_T                = 84,
    SAPP_KEYCODE_U                = 85,
    SAPP_KEYCODE_V                = 86,
    SAPP_KEYCODE_W                = 87,
    SAPP_KEYCODE_X                = 88,
    SAPP_KEYCODE_Y                = 89,
    SAPP_KEYCODE_Z                = 90,
    SAPP_KEYCODE_LEFT_BRACKET     = 91,  /* [ */
    SAPP_KEYCODE_BACKSLASH        = 92,  /* \ */
    SAPP_KEYCODE_RIGHT_BRACKET    = 93,  /* ] */
    SAPP_KEYCODE_GRAVE_ACCENT     = 96,  /* ` */
    SAPP_KEYCODE_WORLD_1          = 161, /* non-US #1 */
    SAPP_KEYCODE_WORLD_2          = 162, /* non-US #2 */
    SAPP_KEYCODE_ESCAPE           = 256,
    SAPP_KEYCODE_ENTER            = 257,
    SAPP_KEYCODE_TAB              = 258,
    SAPP_KEYCODE_BACKSPACE        = 259,
    SAPP_KEYCODE_INSERT           = 260,
    SAPP_KEYCODE_DELETE           = 261,
    SAPP_KEYCODE_RIGHT            = 262,
    SAPP_KEYCODE_LEFT             = 263,
    SAPP_KEYCODE_DOWN             = 264,
    SAPP_KEYCODE_UP               = 265,
    SAPP_KEYCODE_PAGE_UP          = 266,
    SAPP_KEYCODE_PAGE_DOWN        = 267,
    SAPP_KEYCODE_HOME             = 268,
    SAPP_KEYCODE_END              = 269,
    SAPP_KEYCODE_CAPS_LOCK        = 280,
    SAPP_KEYCODE_SCROLL_LOCK      = 281,
    SAPP_KEYCODE_NUM_LOCK         = 282,
    SAPP_KEYCODE_PRINT_SCREEN     = 283,
    SAPP_KEYCODE_PAUSE            = 284,
    SAPP_KEYCODE_F1               = 290,
    SAPP_KEYCODE_F2               = 291,
    SAPP_KEYCODE_F3               = 292,
    SAPP_KEYCODE_F4               = 293,
    SAPP_KEYCODE_F5               = 294,
    SAPP_KEYCODE_F6               = 295,
    SAPP_KEYCODE_F7               = 296,
    SAPP_KEYCODE_F8               = 297,
    SAPP_KEYCODE_F9               = 298,
    SAPP_KEYCODE_F10              = 299,
    SAPP_KEYCODE_F11              = 300,
    SAPP_KEYCODE_F12              = 301,
    SAPP_KEYCODE_F13              = 302,
    SAPP_KEYCODE_F14              = 303,
    SAPP_KEYCODE_F15              = 304,
    SAPP_KEYCODE_F16              = 305,
    SAPP_KEYCODE_F17              = 306,
    SAPP_KEYCODE_F18              = 307,
    SAPP_KEYCODE_F19              = 308,
    SAPP_KEYCODE_F20              = 309,
    SAPP_KEYCODE_F21              = 310,
    SAPP_KEYCODE_F22              = 311,
    SAPP_KEYCODE_F23              = 312,
    SAPP_KEYCODE_F24              = 313,
    SAPP_KEYCODE_F25              = 314,
    SAPP_KEYCODE_KP_0             = 320,
    SAPP_KEYCODE_KP_1             = 321,
    SAPP_KEYCODE_KP_2             = 322,
    SAPP_KEYCODE_KP_3             = 323,
    SAPP_KEYCODE_KP_4             = 324,
    SAPP_KEYCODE_KP_5             = 325,
    SAPP_KEYCODE_KP_6             = 326,
    SAPP_KEYCODE_KP_7             = 327,
    SAPP_KEYCODE_KP_8             = 328,
    SAPP_KEYCODE_KP_9             = 329,
    SAPP_KEYCODE_KP_DECIMAL       = 330,
    SAPP_KEYCODE_KP_DIVIDE        = 331,
    SAPP_KEYCODE_KP_MULTIPLY      = 332,
    SAPP_KEYCODE_KP_SUBTRACT      = 333,
    SAPP_KEYCODE_KP_ADD           = 334,
    SAPP_KEYCODE_KP_ENTER         = 335,
    SAPP_KEYCODE_KP_EQUAL         = 336,
    SAPP_KEYCODE_LEFT_SHIFT       = 340,
    SAPP_KEYCODE_LEFT_CONTROL     = 341,
    SAPP_KEYCODE_LEFT_ALT         = 342,
    SAPP_KEYCODE_LEFT_SUPER       = 343,
    SAPP_KEYCODE_RIGHT_SHIFT      = 344,
    SAPP_KEYCODE_RIGHT_CONTROL    = 345,
    SAPP_KEYCODE_RIGHT_ALT        = 346,
    SAPP_KEYCODE_RIGHT_SUPER      = 347,
    SAPP_KEYCODE_MENU             = 348,
} sapp_keycode;

typedef struct {
    uintptr_t identifier;
    float pos_x;
    float pos_y;
    bool changed;
} sapp_touchpoint;

typedef enum {
    SAPP_MOUSEBUTTON_INVALID = -1,
    SAPP_MOUSEBUTTON_LEFT = 0,
    SAPP_MOUSEBUTTON_RIGHT = 1,
    SAPP_MOUSEBUTTON_MIDDLE = 2,
} sapp_mousebutton;

enum {
    SAPP_MODIFIER_SHIFT = (1<<0),
    SAPP_MODIFIER_CTRL = (1<<1),
    SAPP_MODIFIER_ALT = (1<<2),
    SAPP_MODIFIER_SUPER = (1<<3)
};

typedef struct {
    sapp_event_type type;
    uint32_t frame_count;
    sapp_keycode key_code;
    uint32_t char_code;
    uint32_t modifiers;
    sapp_mousebutton mouse_button;
    float mouse_x;
    float mouse_y;
    float scroll_x;
    float scroll_y;
    int num_touches;
    sapp_touchpoint touches[SAPP_MAX_TOUCHPOINTS];
    int window_width;
    int window_height;
    int framebuffer_width;
    int framebuffer_height;
} sapp_event;

typedef struct {
    void (*init_cb)(void);
    void (*frame_cb)(void);
    void (*cleanup_cb)(void);
    void (*event_cb)(const sapp_event*);
    void (*fail_cb)(const char*);
    int width;
    int height;
    int sample_count;
    int swap_interval;
    bool high_dpi;
    bool fullscreen;
    bool alpha;
    bool premultiplied_alpha;
    bool preserve_drawing_buffer;
    const char* window_title;
    const char* html5_canvas_name;
    bool html5_canvas_resize;
    bool ios_keyboard_resizes_canvas;
    /* use GLES2 even if GLES3 is available */
    bool gl_force_gles2; 
    /* if true, user is expected to manage cursor image and visibility on SAPP_EVENTTYPE_UPDATE_CURSOR */
    bool user_cursor;
} sapp_desc;

/* user-provided functions */
extern sapp_desc sokol_main(int argc, char* argv[]);

/* sokol_app API functions */
SOKOL_API_DECL bool sapp_isvalid(void);
SOKOL_API_DECL int sapp_width(void);
SOKOL_API_DECL int sapp_height(void);
SOKOL_API_DECL bool sapp_high_dpi(void);
SOKOL_API_DECL float sapp_dpi_scale(void);
SOKOL_API_DECL void sapp_show_keyboard(bool visible);
SOKOL_API_DECL bool sapp_keyboard_shown(void);

/* GL/GLES specific functions */
SOKOL_API_DECL bool sapp_gles2(void);

/* OSX/Metal specific functions */
SOKOL_API_DECL const void* sapp_metal_get_device(void);
SOKOL_API_DECL const void* sapp_metal_get_renderpass_descriptor(void);
SOKOL_API_DECL const void* sapp_metal_get_drawable(void); 
SOKOL_API_DECL const void* sapp_macos_get_window(void);
SOKOL_API_DECL const void* sapp_ios_get_window(void);

/* Win32/D3D11 specific functions */
SOKOL_API_DECL const void* sapp_d3d11_get_device(void);
SOKOL_API_DECL const void* sapp_d3d11_get_device_context(void);
SOKOL_API_DECL const void* sapp_d3d11_get_render_target_view(void);
SOKOL_API_DECL const void* sapp_d3d11_get_depth_stencil_view(void);
SOKOL_API_DECL const void* sapp_win32_get_hwnd(void);

#ifdef __cplusplus
} /* extern "C" */
#endif
