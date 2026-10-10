#pragma once

#include <math.h>
#include <stdbool.h>

#include "bmp.h"
#include "math.h"
#include "min.h"

typedef struct {
    pixel_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
    float_t *depth_buffer;
} RenderManager;

typedef struct {
    /// クリッピングするかどうか vertex_shader参照
    bool uses_clip_space;

    size_t vertex_in_size;
    size_t vertex_out_size;

    /**
    - 第1引数: vertex_in (custom per shader)
    - 第2引数: uniforms (custom per shader)
    - 第3引数: vertex_out (output buffer, pre-allocated)
    */
    void (*vertex_shader)(vertex_in_t, uniform_t, vertex_out_t);

    /**
    - 第1引数: vertex_out (interpolated from vertices)
    - 第2引数: depth (view space depth W)
    - 第3引数: uniforms (custom per shader)
    - 戻り値: Color で描画。alpha <= 0 で破棄。
    alpha < 1 は半透明合成(深度は書かない)。alpha >= 1 は不透明(深度を書く)。
    */
    Vec4 (*fragment_shader)(vertex_out_t, float depth, uniform_t);

    /**
    - 第1引数: out (output vertex_out)
    - 第2引数: v1, v2, v3 (vertex_out from 3 vertices)
    - 第4引数: barycentric coordinates [a, b, g]
    */
    void (*interpolate_vertex_out)(void *out, void *v1, void *v2, void *v3, Vec3 bary);

} Shader;

// macros
#define INTERP_FIELD(field) o->field = a->field * bary.x + b->field * bary.y + c->field * bary.z;

#define DEFINE_INTERPOLATE(type, fields)                                                 \
    static void interpolate_##type(void *out, void *v1, void *v2, void *v3, Vec3 bary) { \
        type *o = (type *)out;                                                           \
        type *a = (type *)v1;                                                            \
        type *b = (type *)v2;                                                            \
        type *c = (type *)v3;                                                            \
        fields                                                                           \
    }

#define CREATE_SHADER(name, vertex_fn, fragment_fn, interp_fn, v_in_size, v_out_size, uses_clip) \
    (Shader) {                                                                                   \
        .vertex_shader = (void (*)(vertex_in_t, uniform_t, vertex_out_t))vertex_fn,              \
        .fragment_shader = (Vec4 (*)(vertex_out_t, float depth, uniform_t))fragment_fn,          \
        .interpolate_vertex_out = interp_fn, .vertex_in_size = v_in_size,                        \
        .vertex_out_size = v_out_size, .uses_clip_space = uses_clip,                             \
    }

void render_pixel(RenderManager *mgr, UiVec2 p, pixel_t pixel);
void render_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, float near,
                     vertex_in_t v1, vertex_in_t v2, vertex_in_t v3);

void raster(RenderManager *mgr, Shader *shader, uniform_t uniforms, float x1, float y1, float w1,
            float x2, float y2, float w2, float x3, float y3, float w3, vertex_out_t vo1,
            vertex_out_t vo2, vertex_out_t vo3, bool ident);

void resize_depth_buffer(RenderManager *mgr);
void clear_render_buffers(RenderManager *mgr);

Vec4 texture_sample_nearest(const Texture *t, float u, float v);