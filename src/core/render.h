#pragma once

#include <math.h>
#include <stdbool.h>

#include "bmp.h"
#include "math.h"
#include "min.h"
#include "raster.h"

typedef struct {
    pixel_t *pixel_buffer;
    uint32_t width;
    uint32_t height;
    float_t *depth_buffer;

    // 末尾に追加(既存の位置指定初期化を壊さないため)。未指定(0)なら CULL_NONE
    CullMode cull;
} RenderManager;

/**
    型付き補間関数を生成する

        DEFINE_INTERPOLATE(CubeOut,
            INTERP_FIELD(pos.x) INTERP_FIELD(pos.y) INTERP_FIELD(pos.z) INTERP_FIELD(pos.w)
            INTERP_FIELD(u) INTERP_FIELD(v))

*/
#define INTERP_FIELD(field) o->field = a->field * bary.x + b->field * bary.y + c->field * bary.z;

#define DEFINE_INTERPOLATE(type, fields)                                                        \
    static inline void interpolate_##type(type *o, const type *a, const type *b, const type *c, \
                                          Vec3 bary) {                                          \
        fields                                                                                  \
    }

/**
    shader_template.h が生成する関数のプロトタイプ宣言用。ヘッダに書く:

        DECLARE_RENDER_ELEMENTS(cube, CubeVertex, CubeUniform)
*/
#define DECLARE_RENDER_ELEMENTS(name, V_IN, UNIFORMS)                                     \
    void render_elements_##name(RenderManager *mgr, const UNIFORMS *uniforms, float near, \
                                const V_IN *vertices, const int *indices, size_t index_count);

void resize_depth_buffer(RenderManager *mgr);
void clear_render_buffers(RenderManager *mgr);

Vec4 texture_sample_nearest(const Texture *t, float u, float v);
