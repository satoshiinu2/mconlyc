/*
    include するたびに 1 シェーダ分のレンダラを生成する

    事前に定義するマクロ:
        SHADER_NAME  識別子。生成される関数名の接尾辞になる (render_elements_<SHADER_NAME>)
        V_IN          頂点入力の型
        V_OUT         補間される属性の型(位置は含めない)
        UNIFORMS      uniform の型
        VS_FN        Vec4 (*)(const V_IN *, const UNIFORMS *, V_OUT *out)
                       戻り値が位置(gl_Position 相当)、out に属性を書く
                       USES_CLIP=1: 戻り値は clip space (x, y, z, w)。w >= near でクリップする
                       USES_CLIP=0: 戻り値の x, y は画面座標、w は視点深度(> 0)
        FS_FN        Vec4 (*)(const V_OUT *, float depth, const UNIFORMS *)
        INTERP_FN    void (*)(V_OUT *o, const V_OUT *a, const V_OUT *b, const V_OUT *c, Vec3 bary)
                     (DEFINE_INTERPOLATE が interpolate_<型名> として生成する)
        USES_CLIP    0 または 1

    include の後、これらのマクロは #undef される。
*/

#include "min.h"
#include "render.h"

#if !defined(SHADER_NAME) || !defined(V_IN) || !defined(V_OUT) || !defined(UNIFORMS) || \
    !defined(VS_FN) || !defined(FS_FN) || !defined(INTERP_FN) || !defined(USES_CLIP)
#error "shader_template.h: required macros are not defined"
#endif

// 名前結合は2段マクロにしないと SHADER_NAME が展開されない
#ifndef SHADER_TEMPLATE_CAT
#define SHADER_TEMPLATE_CAT_(a, b) a##b
#define SHADER_TEMPLATE_CAT(a, b) SHADER_TEMPLATE_CAT_(a, b)
#endif
#define FN(n) SHADER_TEMPLATE_CAT(SHADER_TEMPLATE_CAT(n, _), SHADER_NAME)

// 位置と属性を組にした頂点(シェーダごとに生成される型)
typedef struct {
    Vec4 pos;
    V_OUT v;
} FN(Vertex);
#define VERT FN(Vertex)

// 画面空間の三角形をラスタライズ(ピクセルループ本体)
static inline void FN(raster_tri)(RenderManager *mgr, const UNIFORMS *uniforms, const VERT *vo1,
                                  const VERT *vo2, const VERT *vo3) {
    const float w1 = vo1->pos.w;
    const float w2 = vo2->pos.w;
    const float w3 = vo3->pos.w;
    float x1, y1, x2, y2, x3, y3;

#if USES_CLIP
    x1 = vo1->pos.x / w1;
    y1 = vo1->pos.y / w1;
    x2 = vo2->pos.x / w2;
    y2 = vo2->pos.y / w2;
    x3 = vo3->pos.x / w3;
    y3 = vo3->pos.y / w3;
#else
    if (w1 <= 0.0f || w2 <= 0.0f || w3 <= 0.0f) return;
    x1 = vo1->pos.x;
    y1 = vo1->pos.y;
    x2 = vo2->pos.x;
    y2 = vo2->pos.y;
    x3 = vo3->pos.x;
    y3 = vo3->pos.y;
#endif

    SetupResult s;
    if (!raster_setup(&s, mgr->width, mgr->height, mgr->cull, x1, y1, w1, x2, y2, w2, x3, y3, w3))
        return;

    const uint32_t width = mgr->width;
    float *const depth = mgr->depth_buffer;
    pixel_t *const pixels = mgr->pixel_buffer;

    const int minY = (int)s.minY;
    const int maxY = (int)s.maxY;
    const float startX = s.minX + 0.5f;
    const float span = s.maxX - s.minX;

    V_OUT frag;  // 三角形ごとに1個。malloc 不要、サイズはぴったり

    for (int y = minY; y <= maxY; y++) {
        const float yc = (float)y + 0.5f;

        const float a0 = s.ca + s.dax * startX + s.day * yc;
        const float b0 = s.cb + s.dbx * startX + s.dby * yc;
        const float g0 = s.cg + s.dgx * startX + s.dgy * yc;

        // 3つの重心座標がすべて >= 0 になる t の範囲 [t0, t1] を求める
        float t0 = 0.0f;
        float t1 = span;

        if (s.dax > 0.0f) {
            float t = ceilf(-a0 * s.inv_dax - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (s.dax < 0.0f) {
            float t = floorf(-a0 * s.inv_dax + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (a0 < 0.0f) {
            continue;
        }

        if (s.dbx > 0.0f) {
            float t = ceilf(-b0 * s.inv_dbx - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (s.dbx < 0.0f) {
            float t = floorf(-b0 * s.inv_dbx + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (b0 < 0.0f) {
            continue;
        }

        if (s.dgx > 0.0f) {
            float t = ceilf(-g0 * s.inv_dgx - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (s.dgx < 0.0f) {
            float t = floorf(-g0 * s.inv_dgx + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (g0 < 0.0f) {
            continue;
        }

        if (t0 > t1) continue;  // float のまま比較してから int にする(巨大値の変換を避ける)

        const int ts = (int)t0;
        const int te = (int)t1;
        size_t di = (size_t)y * width + (size_t)s.minX + (size_t)ts;

        for (int t = ts; t <= te; t++, di++) {
            const float ft = (float)t;
            const float la = a0 + s.dax * ft;
            const float lb = b0 + s.dbx * ft;
            const float lg = g0 + s.dgx * ft;

            // 画面空間で線形な 1/w
            const float wa = la * s.iw1;
            const float wb = lb * s.iw2;
            const float wg = lg * s.iw3;
            const float invW = wa + wb + wg;

            if (invW <= depth[di]) continue;

            // 透視補正
            const float k = 1.0f / invW;
            const Vec3 bary = {wa * k, wb * k, wg * k};

            INTERP_FN(&frag, &vo1->v, &vo2->v, &vo3->v, bary);
            const Vec4 color = FS_FN(&frag, k, uniforms);

            const float alpha = color.w;
            if (alpha <= 0.0f) continue;

            uint32_t r = to_u8(color.x);
            uint32_t g = to_u8(color.y);
            uint32_t b = to_u8(color.z);

            if (alpha < 1.0f) {
                const pixel_t dst = pixels[di];
                const float ia = 1.0f - alpha;
                r = (uint32_t)((float)r * alpha + (float)(dst & 255u) * ia);
                g = (uint32_t)((float)g * alpha + (float)((dst >> 8) & 255u) * ia);
                b = (uint32_t)((float)b * alpha + (float)((dst >> 16) & 255u) * ia);
            } else {
                depth[di] = invW;  // 不透明のみ深度を書く
            }

            pixels[di] = pack_rgba(r, g, b, 0xFF);
        }
    }
}

// 三角形: クリップ -> ラスタライズ
static inline void FN(draw_triangle)(RenderManager *mgr, const UNIFORMS *uniforms, float near,
                                     const VERT *o1, const VERT *o2, const VERT *o3) {
#if USES_CLIP
    const bool in1 = o1->pos.w >= near;
    const bool in2 = o2->pos.w >= near;
    const bool in3 = o3->pos.w >= near;
    const int count = (int)in1 + (int)in2 + (int)in3;

    if (count == 0) return;

    if (count == 3) {
        FN(raster_tri)(mgr, uniforms, o1, o2, o3);
        return;
    }

    // near 平面クリップ(Sutherland–Hodgman)。出力は最大4頂点
    VERT poly[4];
    size_t n = 0;
    const VERT *src[3] = {o1, o2, o3};

    for (size_t i = 0; i < 3; i++) {
        const VERT *a = src[i];
        const VERT *b = src[(i + 1) % 3];
        const bool ina = a->pos.w >= near;
        const bool inb = b->pos.w >= near;

        if (ina) poly[n++] = *a;

        if (ina != inb) {
            const float t = (near - a->pos.w) / (b->pos.w - a->pos.w);
            VERT *p = &poly[n++];

            // 属性は a と b の線形補間(c は重み 0)
            const Vec3 bary = {1.0f - t, t, 0.0f};
            INTERP_FN(&p->v, &a->v, &b->v, &b->v, bary);

            p->pos.x = a->pos.x + (b->pos.x - a->pos.x) * t;
            p->pos.y = a->pos.y + (b->pos.y - a->pos.y) * t;
            p->pos.z = a->pos.z + (b->pos.z - a->pos.z) * t;
            p->pos.w = near;
        }
    }

    // 内外の切り替わりは 0 回か 2 回なので n <= 4
    assert(n <= 4);

    for (size_t i = 1; i + 1 < n; i++) {
        FN(raster_tri)(mgr, uniforms, &poly[0], &poly[i], &poly[i + 1]);
    }
#else
    (void)near;
    FN(raster_tri)(mgr, uniforms, o1, o2, o3);
#endif
}

// エントリポイント: シェーダごとに1つだけ生成される
void FN(render_elements)(RenderManager *mgr, const UNIFORMS *uniforms, float near,
                         const V_IN *vertices, const int *indices, size_t index_count) {
    for (size_t i = 0; i + 2 < index_count; i += 3) {
        VERT vo[3];
        vo[0].pos = VS_FN(&vertices[indices[i]], uniforms, &vo[0].v);
        vo[1].pos = VS_FN(&vertices[indices[i + 1]], uniforms, &vo[1].v);
        vo[2].pos = VS_FN(&vertices[indices[i + 2]], uniforms, &vo[2].v);

        FN(draw_triangle)(mgr, uniforms, near, &vo[0], &vo[1], &vo[2]);
    }
}

// 次の include のために後始末
#undef VERT
#undef FN
#undef SHADER_NAME
#undef V_IN
#undef V_OUT
#undef UNIFORMS
#undef VS_FN
#undef FS_FN
#undef INTERP_FN
#undef USES_CLIP
