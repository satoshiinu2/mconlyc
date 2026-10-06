#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "min.h"
#include "render.h"

typedef struct {
    float minX;
    float maxX;
    float minY;
    float maxY;

    float ca;
    float dax;
    float day;
    float cb;
    float dbx;
    float dby;
    float cg;
    float dgx;
    float dgy;

    float iw1;
    float iw2;
    float iw3;

    bool is_success;
} SetupResult;

// prototypes
static SetupResult setup_raster(RenderManager *mgr, float x1, float y1, float w1, float x2,
                                float y2, float w2, float x3, float y3, float w3);

static void rasterize_rows(RenderManager *mgr, Shader *shader, uniform_t uniforms, SetupResult *s,
                           float w1, float w2, float w3, vertex_out_t vo1, vertex_out_t vo2,
                           vertex_out_t vo3, bool ident);

static void rasterize_pixel(RenderManager *mgr, Shader *shader, uniform_t uniforms, size_t di,
                            float la, float lb, float lg, float iw1, float iw2, float iw3,
                            vertex_out_t vo1, vertex_out_t vo2, vertex_out_t vo3, bool ident);

void raster(RenderManager *mgr, Shader *shader, uniform_t uniforms, float x1, float y1, float w1,
            float x2, float y2, float w2, float x3, float y3, float w3, vertex_out_t vo1,
            vertex_out_t vo2, vertex_out_t vo3, bool ident) {
    SetupResult setup = setup_raster(mgr, x1, y1, w1, x2, y2, w2, x3, y3, w3);

    if (!setup.is_success) return;

    rasterize_rows(mgr, shader, uniforms, &setup, w1, w2, w3, vo1, vo2, vo3, ident);
}

float signedTriangleArea(float x1, float y1, float x2, float y2, float x3, float y3) {
    return ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) / 2;
}

static SetupResult setup_raster(RenderManager *mgr, float x1, float y1, float w1, float x2,
                                float y2, float w2, float x3, float y3, float w3) {
    float area = signedTriangleArea(x1, y1, x2, y2, x3, y3);

    if (area == 0 || !isfinite(area)) {
        return (SetupResult){.is_success = false};
    }

    float width = mgr->width;
    float height = mgr->height;

    float minXraw = fmin(x1, fmin(x2, x3));
    float minX = fmax(0, floor(minXraw));

    float maxXraw = fmax(x1, fmax(x2, x3));
    float maxX = fmin(width - 1, ceil(maxXraw));

    float minYraw = fmin(y1, fmin(y2, y3));
    float minY = fmax(0, floor(minYraw));

    float maxYraw = fmax(y1, fmax(y2, y3));
    float maxY = fmin(height - 1, ceil(maxYraw));

    if (minX > maxX || minY > maxY) {
        return (SetupResult){.is_success = false};
    }

    float inv = 1 / area;

    // barycentric coordinate A
    float ca = ((x2 * y3 - x3 * y2) / 2) * inv;
    float dax = ((y2 - y3) / 2) * inv;
    float day = ((x3 - x2) / 2) * inv;

    // barycentric coordinate B
    float cb = ((y1 * (x3 - x1) - x1 * (y3 - y1)) / 2) * inv;

    float dbx = ((y3 - y1) / 2) * inv;
    float dby = ((x1 - x3) / 2) * inv;

    // barycentric coordinate C
    float cg = ((x1 * (y2 - y1) - y1 * (x2 - x1)) / 2) * inv;

    float dgx = ((y1 - y2) / 2) * inv;
    float dgy = ((x2 - x1) / 2) * inv;

    return (SetupResult){minX,
                         maxX,
                         minY,
                         maxY,

                         ca,
                         dax,
                         day,
                         cb,
                         dbx,
                         dby,
                         cg,
                         dgx,
                         dgy,

                         1 / w1,  // iw1
                         1 / w2,  // iw2
                         1 / w3,  // iw3

                         .is_success = true};
}

static void rasterize_rows(RenderManager *mgr, Shader *shader, uniform_t uniforms,
                           SetupResult *setup, float w1, float w2, float w3, vertex_out_t vo1,
                           vertex_out_t vo2, vertex_out_t vo3, bool ident) {
    uint32_t width = mgr->width;

    float minX = setup->minX;
    float maxX = setup->maxX;
    float minY = setup->minY;
    float maxY = setup->maxY;

    float ca = setup->ca;
    float dax = setup->dax;
    float day = setup->day;
    float cb = setup->cb;
    float dbx = setup->dbx;
    float dby = setup->dby;
    float cg = setup->cg;
    float dgx = setup->dgx;
    float dgy = setup->dgy;

    float iw1 = setup->iw1;
    float iw2 = setup->iw2;
    float iw3 = setup->iw3;

    float startX = minX + 0.5;

    for (size_t y = minY; y <= maxY; y++) {
        float yc = y + 0.5;

        float a0 = ca + dax * startX + day * yc;
        float b0 = cb + dbx * startX + dby * yc;
        float g0 = cg + dgx * startX + dgy * yc;

        float t0 = 0;
        float t1 = maxX - minX;

        if (dax > 0) {
            float t = ceil(-a0 / dax - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (dax < 0) {
            float t = floor(-a0 / dax + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (a0 < 0) {
            continue;
        }

        if (dbx > 0) {
            float t = ceil(-b0 / dbx - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (dbx < 0) {
            float t = floor(-b0 / dbx + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (b0 < 0) {
            continue;
        }

        if (dgx > 0) {
            float t = ceil(-g0 / dgx - FLT_EPSILON);
            if (t > t0) t0 = t;
        } else if (dgx < 0) {
            float t = floor(-g0 / dgx + FLT_EPSILON);
            if (t < t1) t1 = t;
        } else if (g0 < 0) {
            continue;
        }

        if (t0 > t1) {
            continue;
        }

        float di = y * width + minX + t0;

        for (size_t t = t0; t <= t1; t++, di++) {
            float la = a0 + dax * t;
            float lb = b0 + dbx * t;
            float lg = g0 + dgx * t;

            rasterize_pixel(mgr, shader, uniforms, di,

                            la, lb, lg,

                            iw1, iw2, iw3,

                            vo1, vo2, vo3, ident);
        }
    }
}

static void rasterize_pixel(RenderManager *mgr, Shader *shader, uniform_t uniforms, size_t di,
                            float la, float lb, float lg, float iw1, float iw2, float iw3,
                            vertex_out_t vo1, vertex_out_t vo2, vertex_out_t vo3, bool ident) {
    float_t *depth = mgr->depth_buffer;
    pixel_t *pixels = mgr->pixel_buffer;

    // 画面空間で線形な 1/w
    float wa = la * iw1;
    float wb = lb * iw2;
    float wg = lg * iw3;

    float invW = wa + wb + wg;

    if (invW <= depth[di]) {
        return;
    }

    // 透視補正
    float k = 1 / invW;
    Vec3 bary;

    if (ident) {
        bary.x = wa * k;
        bary.y = wb * k;
        bary.z = wg * k;
    } else {
        // For clipped triangles, we need the original barycentric coords
        // Since we're not passing B1, B2, B3 anymore, we use linear interpolation
        bary.x = wa * k;
        bary.y = wb * k;
        bary.z = wg * k;
    }

    // Interpolate vertex_out
    void *vertex_out = malloc(shader->vertex_out_size);
    if (!vertex_out) {
        return;
    }

    shader->interpolate_vertex_out(vertex_out, vo1, vo2, vo3, bary);

    Vec4 color = shader->fragment_shader(vertex_out, k, uniforms);

    free(vertex_out);

    float alpha = color.w;

    if (alpha <= 0) {
        return;
    }

    int r = color.x * 255;
    int g = color.y * 255;
    int b = color.z * 255;

    if (alpha < 1) {
        pixel_t dst = mgr->pixel_buffer[di];

        r = r * alpha + (dst & 255) * (1 - alpha);
        g = g * alpha + ((dst >> 8) & 255) * (1 - alpha);
        b = b * alpha + ((dst >> 16) & 255) * (1 - alpha);
    } else {
        mgr->depth_buffer[di] = invW;
    }

    pixels[di] = 0xFF000000 | (((uint8_t)b | 0) << 16) | (((uint8_t)g | 0) << 8) | ((uint8_t)r | 0);
}