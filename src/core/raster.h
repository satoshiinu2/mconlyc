#pragma once

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "min.h"
#include "vec.h"

typedef enum {
    CULL_NONE = 0,  // 0 にしておく: RenderManager をゼロ初期化すればカリングなし
    CULL_NEGATIVE,  // signed area < 0 を捨てる
    CULL_POSITIVE,  // signed area > 0 を捨てる
} CullMode;

// 三角形ごとに1回だけ計算する(ピクセルごとではない)ので、非インラインで raster.c に置く
typedef struct {
    float minX, maxX, minY, maxY;

    // バリセントリック座標 A, B, G: f(x, y) = c + dx * x + dy * y
    float ca, dax, day;
    float cb, dbx, dby;
    float cg, dgx, dgy;

    // 逆数(行ごとの除算を乗算にするため)
    float inv_dax, inv_dbx, inv_dgx;

    // 1/w
    float iw1, iw2, iw3;
} SetupResult;

bool raster_setup(SetupResult *s, uint32_t width, uint32_t height, CullMode cull, float x1,
                  float y1, float w1, float x2, float y2, float w2, float x3, float y3, float w3);

// [0,1] にクランプして 0..255 へ
static inline uint32_t to_u8(float v) {
    v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    return (uint32_t)(v * 255.0f + 0.5f);
}
