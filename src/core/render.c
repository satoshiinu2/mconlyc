#include "render.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "math.h"
#include "min.h"
#include "vec.h"

// 三角形のクリップ・ラスタライズは shader_template.h がシェーダごとに生成する

void resize_depth_buffer(RenderManager *mgr) {
    size_t bytes = (size_t)mgr->width * mgr->height * sizeof(float_t);

    mgr->depth_buffer = realloc_or_exit(mgr->depth_buffer, bytes);
}

void clear_render_buffers(RenderManager *mgr) {
    size_t n = (size_t)mgr->width * mgr->height;
    memset(mgr->depth_buffer, 0, n * sizeof(*mgr->depth_buffer));
    memset(mgr->pixel_buffer, 0, n * sizeof(*mgr->pixel_buffer));
}

// direct3d like uv
Vec4 texture_sample_nearest(const Texture *t, float u, float v) {
    if (t->pixels == NULL) {
        bool f = (u > 0.5f) ^ (v > 0.5f);
        return f ? (Vec4){1, 0, 1, 1} : (Vec4){0, 0, 0, 1};
    }

    // repeat ラップ
    u -= floorf(u);
    v -= floorf(v);

    int x = (int)(u * (float)t->width);
    int y = (int)(v * (float)t->height);
    if (x >= (int)t->width) x = (int)t->width - 1;
    if (y >= (int)t->height) y = (int)t->height - 1;

    pixel_t p = t->pixels[(size_t)y * t->width + x];
    return rgba8_to_vec4(p);
}
