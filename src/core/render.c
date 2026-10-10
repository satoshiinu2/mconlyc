#include "render.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "math.h"
#include "min.h"
#include "vec.h"

typedef struct {
    float x;
    float y;
    float w;

    Vec3 bary;
    void *vertex_out;

    // クリップで新規 malloc したものだけ true
    bool owned;
} ClipVertex;

// const
static const Vec3 BARY_V0 = {1, 0, 0};
static const Vec3 BARY_V1 = {0, 1, 0};
static const Vec3 BARY_V2 = {0, 0, 1};

// prototypes
static void draw_clip_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, float near,
                               Vec4 o1, Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                               vertex_out_t vo3);
static void draw_screen_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, Vec4 o1,
                                 Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                                 vertex_out_t vo3);
static void draw_clip_space_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms,
                                     Vec4 o1, Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                                     vertex_out_t vo3);
// out_ptr must be empty. its contents are overwritten, not freed
static size_t clip_triangle_near(Shader *shader, ClipVertex out[4], float near, Vec4 o1, Vec4 o2,
                                 Vec4 o3, vertex_out_t vo1, vertex_out_t vo2, vertex_out_t vo3);
static void rasterize_polygon(RenderManager *mgr, Shader *shader, uniform_t uniforms,
                              ClipVertex polygon[4], size_t polygon_len);

void render_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, float near,
                     vertex_in_t v1, vertex_in_t v2, vertex_in_t v3) {
    void *vo1 = malloc(shader->vertex_out_size);
    void *vo2 = malloc(shader->vertex_out_size);
    void *vo3 = malloc(shader->vertex_out_size);

    if (!vo1 || !vo2 || !vo3) {
        if (vo1) free(vo1);
        if (vo2) free(vo2);
        if (vo3) free(vo3);
        return;
    }

    shader->vertex_shader(v1, uniforms, vo1);
    shader->vertex_shader(v2, uniforms, vo2);
    shader->vertex_shader(v3, uniforms, vo3);

    Vec4 o1 = *(Vec4 *)vo1;
    Vec4 o2 = *(Vec4 *)vo2;
    Vec4 o3 = *(Vec4 *)vo3;

    if (shader->uses_clip_space) {
        draw_clip_triangle(mgr, shader, uniforms, near, o1, o2, o3, vo1, vo2, vo3);
    } else {
        draw_screen_triangle(mgr, shader, uniforms, o1, o2, o3, vo1, vo2, vo3);
    }

    free(vo1);
    free(vo2);
    free(vo3);
}

static void draw_screen_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, Vec4 o1,
                                 Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                                 vertex_out_t vo3) {
    if (o1.z <= 0 || o2.z <= 0 || o3.z <= 0) {
        return;
    }

    raster(mgr, shader, uniforms, o1.x, o1.y, o1.z, o2.x, o2.y, o2.z, o3.x, o3.y, o3.z, vo1, vo2,
           vo3, true);
}

static void draw_clip_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms, float near,
                               Vec4 o1, Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                               vertex_out_t vo3) {
    bool in1 = o1.w >= near;
    bool in2 = o2.w >= near;
    bool in3 = o3.w >= near;

    int count = in1 + in2 + in3;

    if (count == 0) {
        return;
    }

    if (count == 3) {
        draw_clip_space_triangle(mgr, shader, uniforms, o1, o2, o3, vo1, vo2, vo3);
        return;
    }

    ClipVertex polygon[4];
    size_t len = clip_triangle_near(shader, polygon, near, o1, o2, o3, vo1, vo2, vo3);
    rasterize_polygon(mgr, shader, uniforms, polygon, len);
}

static void draw_clip_space_triangle(RenderManager *mgr, Shader *shader, uniform_t uniforms,
                                     Vec4 o1, Vec4 o2, Vec4 o3, vertex_out_t vo1, vertex_out_t vo2,
                                     vertex_out_t vo3) {
    float w1 = o1.w;
    float w2 = o2.w;
    float w3 = o3.w;

    float x1 = o1.x / w1;
    float y1 = o1.y / w1;
    float x2 = o2.x / w2;
    float y2 = o2.y / w2;
    float x3 = o3.x / w3;
    float y3 = o3.y / w3;

    raster(mgr, shader, uniforms, x1, y1, w1, x2, y2, w2, x3, y3, w3, vo1, vo2, vo3, true);
}

static size_t clip_triangle_near(Shader *shader, ClipVertex out[4], float near, Vec4 o1, Vec4 o2,
                                 Vec4 o3, vertex_out_t vo1, vertex_out_t vo2, vertex_out_t vo3) {
    const ClipVertex poly[3] = {
        {o1.x, o1.y, o1.w, BARY_V0, vo1, false},
        {o2.x, o2.y, o2.w, BARY_V1, vo2, false},
        {o3.x, o3.y, o3.w, BARY_V2, vo3, false},
    };

    size_t n = 0;

    for (size_t i = 0; i < 3; i++) {
        ClipVertex a = poly[i];
        ClipVertex b = poly[(i + 1) % 3];

        bool ina = a.w >= near;
        bool inb = b.w >= near;

        if (ina) out[n++] = a;

        if (ina != inb) {
            float t = (near - a.w) / (b.w - a.w);

            ClipVertex p = {
                .x = a.x + (b.x - a.x) * t,
                .y = a.y + (b.y - a.y) * t,
                .w = near,
                .bary =
                    {
                        a.bary.x + (b.bary.x - a.bary.x) * t,
                        a.bary.y + (b.bary.y - a.bary.y) * t,
                        a.bary.z + (b.bary.z - a.bary.z) * t,
                    },
                .vertex_out = malloc_or_exit(shader->vertex_out_size),
                .owned = true,
            };

            shader->interpolate_vertex_out(p.vertex_out, vo1, vo2, vo3, p.bary);

            out[n++] = p;
        }
    }

    // safety: nは必ず4以下です
    /*
        出力される頂点は「内側にある元頂点」と「内外が切り替わる辺の交点」の合計です。

        三角形の3辺を一周すると、内外の切り替わりは 0 回か 2
        回のどちらかです。閉じたループなので、奇数回にはなりません。 よって頂点数は「内側の頂点数 +
        交点数」になります。
    */
    assert(n <= 4);

    return n;
}

static void rasterize_polygon(RenderManager *mgr, Shader *shader, void *uniforms,
                              ClipVertex polygon[4], size_t polygon_len) {
    for (size_t i = 1; i < polygon_len - 1; i++) {
        ClipVertex a = polygon[0];
        ClipVertex b = polygon[i];
        ClipVertex c = polygon[i + 1];

        float x1 = a.x / a.w;
        float y1 = a.y / a.w;
        float w1 = a.w;
        float x2 = b.x / b.w;
        float y2 = b.y / b.w;
        float w2 = b.w;
        float x3 = c.x / c.w;
        float y3 = c.y / c.w;
        float w3 = c.w;

        raster(mgr, shader, uniforms, x1, y1, w1, x2, y2, w2, x3, y3, w3, a.vertex_out,
               b.vertex_out, c.vertex_out, false);
    }

    // ownedならfree
    for (size_t i = 0; i < polygon_len; i++) {
        if (polygon[i].owned) free(polygon[i].vertex_out);
    }
}

void render_elements(RenderManager *mgr, Shader *shader, uniform_t uniforms, float near,
                     vertex_in_t *vertices, size_t stride, const int *indices, size_t indexCount) {
    const char *base = (const char *)vertices;

    for (size_t i = 0; i + 2 < indexCount; i += 3) {
        render_triangle(mgr, shader, uniforms, near,
                        (vertex_in_t)(base + (size_t)indices[i] * stride),
                        (vertex_in_t)(base + (size_t)indices[i + 1] * stride),
                        (vertex_in_t)(base + (size_t)indices[i + 2] * stride));
    }
}

void resize_depth_buffer(RenderManager *mgr) {
    size_t byte = mgr->width * mgr->height * sizeof(float);

    mgr->depth_buffer = realloc_or_exit(mgr->depth_buffer, byte);
}

void clear_render_buffers(RenderManager *mgr) {
    size_t n = (size_t)mgr->width * mgr->height;
    memset(mgr->depth_buffer, 0, n * sizeof(*mgr->depth_buffer));
    memset(mgr->pixel_buffer, 0, n * sizeof(*mgr->pixel_buffer));
}

// direct3d like uv
Vec4 texture_sample_nearest(const Texture *t, float u, float v) {
    if (t->pixels == NULL) {
        bool f = (u > 0.5) ^ (v > 0.5);
        return f ? (Vec4){1, 0, 1, 1} : (Vec4){0, 0, 0, 1};
    }

    // repeat ラップ
    u -= floorf(u);
    v -= floorf(v);

    int x = (int)(u * t->width);
    int y = (int)(v * t->height);
    if (x >= t->width) x = t->width - 1;
    if (y >= t->height) y = t->height - 1;

    pixel_t p = t->pixels[y * t->width + x];
    return rgba8_to_vec4(p);
}