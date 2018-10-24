#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifndef SOKOL_API_DECL
    #define SOKOL_API_DECL extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4201)   /* nonstandard extension used: nameless struct/union */
#endif

typedef struct { uint32_t id; } sg_buffer;
typedef struct { uint32_t id; } sg_image;
typedef struct { uint32_t id; } sg_shader;
typedef struct { uint32_t id; } sg_pipeline;
typedef struct { uint32_t id; } sg_pass;
typedef struct { uint32_t id; } sg_context;

enum {
    SG_INVALID_ID = 0,
    SG_NUM_SHADER_STAGES = 2,
    SG_NUM_INFLIGHT_FRAMES = 2,
    SG_MAX_COLOR_ATTACHMENTS = 4,
    SG_MAX_SHADERSTAGE_BUFFERS = 4,
    SG_MAX_SHADERSTAGE_IMAGES = 12,
    SG_MAX_SHADERSTAGE_UBS = 4,
    SG_MAX_UB_MEMBERS = 16,
    SG_MAX_VERTEX_ATTRIBUTES = 16,
    SG_MAX_MIPMAPS = 16,
    SG_MAX_TEXTUREARRAY_LAYERS = 128
};

typedef enum {
    SG_FEATURE_INSTANCING,
    SG_FEATURE_TEXTURE_COMPRESSION_DXT,
    SG_FEATURE_TEXTURE_COMPRESSION_PVRTC,
    SG_FEATURE_TEXTURE_COMPRESSION_ATC,
    SG_FEATURE_TEXTURE_COMPRESSION_ETC2,
    SG_FEATURE_TEXTURE_FLOAT,
    SG_FEATURE_TEXTURE_HALF_FLOAT,
    SG_FEATURE_ORIGIN_BOTTOM_LEFT,
    SG_FEATURE_ORIGIN_TOP_LEFT,
    SG_FEATURE_MSAA_RENDER_TARGETS,
    SG_FEATURE_PACKED_VERTEX_FORMAT_10_2,
    SG_FEATURE_MULTIPLE_RENDER_TARGET,
    SG_FEATURE_IMAGETYPE_3D,
    SG_FEATURE_IMAGETYPE_ARRAY,

    SG_NUM_FEATURES
} sg_feature;

typedef enum {
    SG_RESOURCESTATE_INITIAL,
    SG_RESOURCESTATE_ALLOC,
    SG_RESOURCESTATE_VALID,
    SG_RESOURCESTATE_FAILED,
    SG_RESOURCESTATE_INVALID,
    _SG_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} sg_resource_state;

typedef enum {
    _SG_USAGE_DEFAULT,      /* value 0 reserved for default-init */
    SG_USAGE_IMMUTABLE,
    SG_USAGE_DYNAMIC,
    SG_USAGE_STREAM,
    _SG_USAGE_NUM,
    _SG_USAGE_FORCE_U32 = 0x7FFFFFFF
} sg_usage;

typedef enum {
    _SG_BUFFERTYPE_DEFAULT,         /* value 0 reserved for default-init */
    SG_BUFFERTYPE_VERTEXBUFFER,
    SG_BUFFERTYPE_INDEXBUFFER,
    _SG_BUFFERTYPE_NUM,
    _SG_BUFFERTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_buffer_type;

typedef enum {
    _SG_INDEXTYPE_DEFAULT,   /* value 0 reserved for default-init */
    SG_INDEXTYPE_NONE,
    SG_INDEXTYPE_UINT16,
    SG_INDEXTYPE_UINT32,
    _SG_INDEXTYPE_NUM,
    _SG_INDEXTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_index_type;

typedef enum {
    _SG_IMAGETYPE_DEFAULT,  /* value 0 reserved for default-init */
    SG_IMAGETYPE_2D,
    SG_IMAGETYPE_CUBE,
    SG_IMAGETYPE_3D,
    SG_IMAGETYPE_ARRAY,
    _SG_IMAGETYPE_NUM,
    _SG_IMAGETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_image_type;

typedef enum {
    SG_CUBEFACE_POS_X,
    SG_CUBEFACE_NEG_X,
    SG_CUBEFACE_POS_Y,
    SG_CUBEFACE_NEG_Y,
    SG_CUBEFACE_POS_Z,
    SG_CUBEFACE_NEG_Z,
    SG_CUBEFACE_NUM,
    _SG_CUBEFACE_FORCE_U32 = 0x7FFFFFFF
} sg_cube_face;

typedef enum {
    SG_SHADERSTAGE_VS,
    SG_SHADERSTAGE_FS,
    _SG_SHADERSTAGE_FORCE_U32 = 0x7FFFFFFF
} sg_shader_stage;

typedef enum {
    _SG_PIXELFORMAT_DEFAULT,    /* value 0 reserved for default-init */
    SG_PIXELFORMAT_NONE,
    SG_PIXELFORMAT_RGBA8,
    SG_PIXELFORMAT_RGB8,
    SG_PIXELFORMAT_RGBA4,
    SG_PIXELFORMAT_R5G6B5,
    SG_PIXELFORMAT_R5G5B5A1,
    SG_PIXELFORMAT_R10G10B10A2,
    SG_PIXELFORMAT_RGBA32F,
    SG_PIXELFORMAT_RGBA16F,
    SG_PIXELFORMAT_R32F,
    SG_PIXELFORMAT_R16F,
    SG_PIXELFORMAT_L8,
    SG_PIXELFORMAT_DXT1,
    SG_PIXELFORMAT_DXT3,
    SG_PIXELFORMAT_DXT5,
    SG_PIXELFORMAT_DEPTH,
    SG_PIXELFORMAT_DEPTHSTENCIL,
    SG_PIXELFORMAT_PVRTC2_RGB,
    SG_PIXELFORMAT_PVRTC4_RGB,
    SG_PIXELFORMAT_PVRTC2_RGBA,
    SG_PIXELFORMAT_PVRTC4_RGBA,
    SG_PIXELFORMAT_ETC2_RGB8,
    SG_PIXELFORMAT_ETC2_SRGB8,
    _SG_PIXELFORMAT_NUM,
    _SG_PIXELFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_pixel_format;

typedef enum {
    _SG_PRIMITIVETYPE_DEFAULT,  /* value 0 reserved for default-init */
    SG_PRIMITIVETYPE_POINTS,
    SG_PRIMITIVETYPE_LINES,
    SG_PRIMITIVETYPE_LINE_STRIP,
    SG_PRIMITIVETYPE_TRIANGLES,
    SG_PRIMITIVETYPE_TRIANGLE_STRIP,
    _SG_PRIMITIVETYPE_NUM,
    _SG_PRIMITIVETYPE_FORCE_U32 = 0x7FFFFFFF
} sg_primitive_type;

typedef enum {
    _SG_FILTER_DEFAULT, /* value 0 reserved for default-init */
    SG_FILTER_NEAREST,
    SG_FILTER_LINEAR,
    SG_FILTER_NEAREST_MIPMAP_NEAREST,
    SG_FILTER_NEAREST_MIPMAP_LINEAR,
    SG_FILTER_LINEAR_MIPMAP_NEAREST,
    SG_FILTER_LINEAR_MIPMAP_LINEAR,
    _SG_FILTER_NUM,
    _SG_FILTER_FORCE_U32 = 0x7FFFFFFF
} sg_filter;

typedef enum {
    _SG_WRAP_DEFAULT,   /* value 0 reserved for default-init */
    SG_WRAP_REPEAT,
    SG_WRAP_CLAMP_TO_EDGE,
    SG_WRAP_MIRRORED_REPEAT,
    _SG_WRAP_NUM,
    _SG_WRAP_FORCE_U32 = 0x7FFFFFFF
} sg_wrap;

typedef enum {
    SG_VERTEXFORMAT_INVALID,
    SG_VERTEXFORMAT_FLOAT,
    SG_VERTEXFORMAT_FLOAT2,
    SG_VERTEXFORMAT_FLOAT3,
    SG_VERTEXFORMAT_FLOAT4,
    SG_VERTEXFORMAT_BYTE4,
    SG_VERTEXFORMAT_BYTE4N,
    SG_VERTEXFORMAT_UBYTE4,
    SG_VERTEXFORMAT_UBYTE4N,
    SG_VERTEXFORMAT_SHORT2,
    SG_VERTEXFORMAT_SHORT2N,
    SG_VERTEXFORMAT_SHORT4,
    SG_VERTEXFORMAT_SHORT4N,
    SG_VERTEXFORMAT_UINT10_N2,
    _SG_VERTEXFORMAT_NUM,
    _SG_VERTEXFORMAT_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_format;

typedef enum {
    _SG_VERTEXSTEP_DEFAULT,     /* value 0 reserved for default-init */
    SG_VERTEXSTEP_PER_VERTEX,
    SG_VERTEXSTEP_PER_INSTANCE,
    _SG_VERTEXSTEP_NUM,
    _SG_VERTEXSTEP_FORCE_U32 = 0x7FFFFFFF
} sg_vertex_step;

typedef enum {
    SG_UNIFORMTYPE_INVALID,
    SG_UNIFORMTYPE_FLOAT,
    SG_UNIFORMTYPE_FLOAT2,
    SG_UNIFORMTYPE_FLOAT3,
    SG_UNIFORMTYPE_FLOAT4,
    SG_UNIFORMTYPE_MAT4,
    _SG_UNIFORMTYPE_NUM,
    _SG_UNIFORMTYPE_FORCE_U32 = 0x7FFFFFFF
} sg_uniform_type;

typedef enum {
    _SG_CULLMODE_DEFAULT,   /* value 0 reserved for default-init */
    SG_CULLMODE_NONE,
    SG_CULLMODE_FRONT,
    SG_CULLMODE_BACK,
    _SG_CULLMODE_NUM,
    _SG_CULLMODE_FORCE_U32 = 0x7FFFFFFF
} sg_cull_mode;

typedef enum {
    _SG_FACEWINDING_DEFAULT,    /* value 0 reserved for default-init */
    SG_FACEWINDING_CCW,
    SG_FACEWINDING_CW,
    _SG_FACEWINDING_NUM,
    _SG_FACEWINDING_FORCE_U32 = 0x7FFFFFFF
} sg_face_winding;

typedef enum {
    _SG_COMPAREFUNC_DEFAULT,    /* value 0 reserved for default-init */
    SG_COMPAREFUNC_NEVER,
    SG_COMPAREFUNC_LESS,
    SG_COMPAREFUNC_EQUAL,
    SG_COMPAREFUNC_LESS_EQUAL,
    SG_COMPAREFUNC_GREATER,
    SG_COMPAREFUNC_NOT_EQUAL,
    SG_COMPAREFUNC_GREATER_EQUAL,
    SG_COMPAREFUNC_ALWAYS,
    _SG_COMPAREFUNC_NUM,
    _SG_COMPAREFUNC_FORCE_U32 = 0x7FFFFFFF
} sg_compare_func;

typedef enum {
    _SG_STENCILOP_DEFAULT,      /* value 0 reserved for default-init */
    SG_STENCILOP_KEEP,
    SG_STENCILOP_ZERO,
    SG_STENCILOP_REPLACE,
    SG_STENCILOP_INCR_CLAMP,
    SG_STENCILOP_DECR_CLAMP,
    SG_STENCILOP_INVERT,
    SG_STENCILOP_INCR_WRAP,
    SG_STENCILOP_DECR_WRAP,
    _SG_STENCILOP_NUM,
    _SG_STENCILOP_FORCE_U32 = 0x7FFFFFFF
} sg_stencil_op;

typedef enum {
    _SG_BLENDFACTOR_DEFAULT,    /* value 0 reserved for default-init */
    SG_BLENDFACTOR_ZERO,
    SG_BLENDFACTOR_ONE,
    SG_BLENDFACTOR_SRC_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR,
    SG_BLENDFACTOR_SRC_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
    SG_BLENDFACTOR_DST_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_DST_COLOR,
    SG_BLENDFACTOR_DST_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA,
    SG_BLENDFACTOR_SRC_ALPHA_SATURATED,
    SG_BLENDFACTOR_BLEND_COLOR,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_COLOR,
    SG_BLENDFACTOR_BLEND_ALPHA,
    SG_BLENDFACTOR_ONE_MINUS_BLEND_ALPHA,
    _SG_BLENDFACTOR_NUM,
    _SG_BLENDFACTOR_FORCE_U32 = 0x7FFFFFFF
} sg_blend_factor;

typedef enum {
    _SG_BLENDOP_DEFAULT,    /* value 0 reserved for default-init */
    SG_BLENDOP_ADD,
    SG_BLENDOP_SUBTRACT,
    SG_BLENDOP_REVERSE_SUBTRACT,
    _SG_BLENDOP_NUM,
    _SG_BLENDOP_FORCE_U32 = 0x7FFFFFFF
} sg_blend_op;

typedef enum {
    _SG_COLORMASK_DEFAULT = 0,      /* value 0 reserved for default-init */
    SG_COLORMASK_NONE = (0x10),     /* special value for 'all channels disabled */
    SG_COLORMASK_R = (1<<0),
    SG_COLORMASK_G = (1<<1),
    SG_COLORMASK_B = (1<<2),
    SG_COLORMASK_A = (1<<3),
    SG_COLORMASK_RGB = 0x7,
    SG_COLORMASK_RGBA = 0xF,
    _SG_COLORMASK_FORCE_U32 = 0x7FFFFFFF
} sg_color_mask;

typedef enum {
    _SG_ACTION_DEFAULT,
    SG_ACTION_CLEAR,
    SG_ACTION_LOAD,
    SG_ACTION_DONTCARE,
    _SG_ACTION_NUM,
    _SG_ACTION_FORCE_U32 = 0x7FFFFFFF
} sg_action;

typedef struct {
    sg_action action;
    float val[4];
} sg_color_attachment_action;

typedef struct {
    sg_action action;
    float val;
} sg_depth_attachment_action;

typedef struct {
    sg_action action;
    uint8_t val;
} sg_stencil_attachment_action;

typedef struct {
    uint32_t _start_canary;
    sg_color_attachment_action colors[SG_MAX_COLOR_ATTACHMENTS];
    sg_depth_attachment_action depth;
    sg_stencil_attachment_action stencil;
    uint32_t _end_canary;
} sg_pass_action;

typedef struct {
    uint32_t _start_canary;
    sg_pipeline pipeline;
    sg_buffer vertex_buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    uint32_t vertex_buffer_offsets[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_buffer index_buffer;
    uint32_t index_buffer_offset;
    sg_image vs_images[SG_MAX_SHADERSTAGE_IMAGES];
    sg_image fs_images[SG_MAX_SHADERSTAGE_IMAGES];
    uint32_t _end_canary;
} sg_draw_state;

typedef struct {
    uint32_t _start_canary;
    int buffer_pool_size;
    int image_pool_size;
    int shader_pool_size;
    int pipeline_pool_size;
    int pass_pool_size;
    int context_pool_size;
    /* GL specific */
    bool gl_force_gles2;
    /* Metal-specific */
    const void* mtl_device;
    const void* (*mtl_renderpass_descriptor_cb)(void);
    const void* (*mtl_drawable_cb)(void);
    int mtl_global_uniform_buffer_size;
    int mtl_sampler_cache_size;
    /* D3D11-specific */
    const void* d3d11_device;
    const void* d3d11_device_context;
    const void* (*d3d11_render_target_view_cb)(void);
    const void* (*d3d11_depth_stencil_view_cb)(void);
    uint32_t _end_canary;
} sg_desc;

typedef struct {
    uint32_t _start_canary;
    int size;
    sg_buffer_type type;
    sg_usage usage;
    const void* content;
    /* GL specific */
    uint32_t gl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    const void* mtl_buffers[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void* d3d11_buffer;
    uint32_t _end_canary;
} sg_buffer_desc;

typedef struct {
    const void* ptr;    /* pointer to subimage data */
    int size;           /* size in bytes of pointed-to subimage data */
} sg_subimage_content;

typedef struct {
    sg_subimage_content subimage[SG_CUBEFACE_NUM][SG_MAX_MIPMAPS];
} sg_image_content;

typedef struct {
    uint32_t _start_canary;
    sg_image_type type;
    bool render_target;
    int width;
    int height;
    union {
        int depth;
        int layers;
    };
    int num_mipmaps;
    sg_usage usage;
    sg_pixel_format pixel_format;
    int sample_count;
    sg_filter min_filter;
    sg_filter mag_filter;
    sg_wrap wrap_u;
    sg_wrap wrap_v;
    sg_wrap wrap_w;
    uint32_t max_anisotropy;
    float min_lod;
    float max_lod;
    sg_image_content content;
    /* GL specific */
    uint32_t gl_textures[SG_NUM_INFLIGHT_FRAMES];
    /* Metal specific */
    const void* mtl_textures[SG_NUM_INFLIGHT_FRAMES];
    /* D3D11 specific */
    const void* d3d11_texture;
    uint32_t _end_canary;
} sg_image_desc;

typedef struct {
    const char* name;
    sg_uniform_type type;
    int array_count;
} sg_shader_uniform_desc;

typedef struct {
    int size;
    sg_shader_uniform_desc uniforms[SG_MAX_UB_MEMBERS];
} sg_shader_uniform_block_desc;

typedef struct {
    const char* name;
    sg_image_type type;
} sg_shader_image_desc;

typedef struct {
    const char* source;
    const uint8_t* byte_code;
    int byte_code_size;
    const char* entry;
    sg_shader_uniform_block_desc uniform_blocks[SG_MAX_SHADERSTAGE_UBS];
    sg_shader_image_desc images[SG_MAX_SHADERSTAGE_IMAGES];
} sg_shader_stage_desc;

typedef struct {
    uint32_t _start_canary;
    sg_shader_stage_desc vs;
    sg_shader_stage_desc fs;
    uint32_t _end_canary;
} sg_shader_desc;

typedef struct {
    int stride;
    sg_vertex_step step_func;
    int step_rate;
} sg_buffer_layout_desc;

typedef struct {
    const char* name;
    const char* sem_name;
    int sem_index;
    int buffer_index;
    int offset;
    sg_vertex_format format;
} sg_vertex_attr_desc;

typedef struct {
    sg_buffer_layout_desc buffers[SG_MAX_SHADERSTAGE_BUFFERS];
    sg_vertex_attr_desc attrs[SG_MAX_VERTEX_ATTRIBUTES];
} sg_layout_desc;

typedef struct {
    sg_stencil_op fail_op;
    sg_stencil_op depth_fail_op;
    sg_stencil_op pass_op;
    sg_compare_func compare_func;
} sg_stencil_state;

typedef struct {
    sg_stencil_state stencil_front;
    sg_stencil_state stencil_back;
    sg_compare_func depth_compare_func;
    bool depth_write_enabled;
    bool stencil_enabled;
    uint8_t stencil_read_mask;
    uint8_t stencil_write_mask;
    uint8_t stencil_ref;
} sg_depth_stencil_state;

typedef struct {
    bool enabled;
    sg_blend_factor src_factor_rgb;
    sg_blend_factor dst_factor_rgb;
    sg_blend_op op_rgb;
    sg_blend_factor src_factor_alpha;
    sg_blend_factor dst_factor_alpha;
    sg_blend_op op_alpha;
    uint8_t color_write_mask;
    int color_attachment_count;
    sg_pixel_format color_format;
    sg_pixel_format depth_format;
    float blend_color[4];
} sg_blend_state;

typedef struct {
    bool alpha_to_coverage_enabled;
    sg_cull_mode cull_mode;
    sg_face_winding face_winding;
    int sample_count;
    float depth_bias;
    float depth_bias_slope_scale;
    float depth_bias_clamp;
} sg_rasterizer_state;

typedef struct {
    uint32_t _start_canary;
    sg_layout_desc layout;
    sg_shader shader;
    sg_primitive_type primitive_type;
    sg_index_type index_type;
    sg_depth_stencil_state depth_stencil;
    sg_blend_state blend;
    sg_rasterizer_state rasterizer;
    uint32_t _end_canary;
} sg_pipeline_desc;

typedef struct {
    sg_image image;
    int mip_level;
    union {
        int face;
        int layer;
        int slice;
    };
} sg_attachment_desc;

typedef struct {
    uint32_t _start_canary;
    sg_attachment_desc color_attachments[SG_MAX_COLOR_ATTACHMENTS];
    sg_attachment_desc depth_stencil_attachment;
    uint32_t _end_canary;
} sg_pass_desc;

/* setup and misc functions */
SOKOL_API_DECL void sg_setup(const sg_desc* desc);
SOKOL_API_DECL void sg_shutdown(void);
SOKOL_API_DECL bool sg_isvalid(void);
SOKOL_API_DECL bool sg_query_feature(sg_feature feature);
SOKOL_API_DECL void sg_reset_state_cache(void);

/* resource creation, destruction and updating */
SOKOL_API_DECL sg_buffer sg_make_buffer(const sg_buffer_desc* desc);
SOKOL_API_DECL sg_image sg_make_image(const sg_image_desc* desc);
SOKOL_API_DECL sg_shader sg_make_shader(const sg_shader_desc* desc);
SOKOL_API_DECL sg_pipeline sg_make_pipeline(const sg_pipeline_desc* desc);
SOKOL_API_DECL sg_pass sg_make_pass(const sg_pass_desc* desc);
SOKOL_API_DECL void sg_destroy_buffer(sg_buffer buf);
SOKOL_API_DECL void sg_destroy_image(sg_image img);
SOKOL_API_DECL void sg_destroy_shader(sg_shader shd);
SOKOL_API_DECL void sg_destroy_pipeline(sg_pipeline pip);
SOKOL_API_DECL void sg_destroy_pass(sg_pass pass);
SOKOL_API_DECL void sg_update_buffer(sg_buffer buf, const void* data_ptr, int data_size);
SOKOL_API_DECL void sg_update_image(sg_image img, const sg_image_content* data);

/* get resource state (initial, alloc, valid, failed) */
SOKOL_API_DECL sg_resource_state sg_query_buffer_state(sg_buffer buf);
SOKOL_API_DECL sg_resource_state sg_query_image_state(sg_image img);
SOKOL_API_DECL sg_resource_state sg_query_shader_state(sg_shader shd);
SOKOL_API_DECL sg_resource_state sg_query_pipeline_state(sg_pipeline pip);
SOKOL_API_DECL sg_resource_state sg_query_pass_state(sg_pass pass);

/* rendering functions */
SOKOL_API_DECL void sg_begin_default_pass(const sg_pass_action* pass_action, int width, int height);
SOKOL_API_DECL void sg_begin_pass(sg_pass pass, const sg_pass_action* pass_action);
SOKOL_API_DECL void sg_apply_viewport(int x, int y, int width, int height, bool origin_top_left);
SOKOL_API_DECL void sg_apply_scissor_rect(int x, int y, int width, int height, bool origin_top_left);
SOKOL_API_DECL void sg_apply_draw_state(const sg_draw_state* ds);
SOKOL_API_DECL void sg_apply_uniform_block(sg_shader_stage stage, int ub_index, const void* data, int num_bytes);
SOKOL_API_DECL void sg_draw(int base_element, int num_elements, int num_instances);
SOKOL_API_DECL void sg_end_pass(void);
SOKOL_API_DECL void sg_commit(void);

/* separate resource allocation and initialization (for async setup) */
SOKOL_API_DECL sg_buffer sg_alloc_buffer(void);
SOKOL_API_DECL sg_image sg_alloc_image(void);
SOKOL_API_DECL sg_shader sg_alloc_shader(void);
SOKOL_API_DECL sg_pipeline sg_alloc_pipeline(void);
SOKOL_API_DECL sg_pass sg_alloc_pass(void);
SOKOL_API_DECL void sg_init_buffer(sg_buffer buf_id, const sg_buffer_desc* desc);
SOKOL_API_DECL void sg_init_image(sg_image img_id, const sg_image_desc* desc);
SOKOL_API_DECL void sg_init_shader(sg_shader shd_id, const sg_shader_desc* desc);
SOKOL_API_DECL void sg_init_pipeline(sg_pipeline pip_id, const sg_pipeline_desc* desc);
SOKOL_API_DECL void sg_init_pass(sg_pass pass_id, const sg_pass_desc* desc);
SOKOL_API_DECL void sg_fail_buffer(sg_buffer buf_id);
SOKOL_API_DECL void sg_fail_image(sg_image img_id);
SOKOL_API_DECL void sg_fail_shader(sg_shader shd_id);
SOKOL_API_DECL void sg_fail_pipeline(sg_pipeline pip_id);
SOKOL_API_DECL void sg_fail_pass(sg_pass pass_id);

/* rendering contexts (optional) */
SOKOL_API_DECL sg_context sg_setup_context(void);
SOKOL_API_DECL void sg_activate_context(sg_context ctx_id);
SOKOL_API_DECL void sg_discard_context(sg_context ctx_id);

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif
