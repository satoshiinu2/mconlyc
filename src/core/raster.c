#include "raster.h"

static float signed_triangle_area(float x1, float y1, float x2, float y2, float x3, float y3) {
    return ((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) * 0.5f;
}

bool raster_setup(SetupResult *s, uint32_t width, uint32_t height, CullMode cull, float x1,
                  float y1, float w1, float x2, float y2, float w2, float x3, float y3, float w3) {
    float area = signed_triangle_area(x1, y1, x2, y2, x3, y3);

    if (area == 0.0f || !isfinite(area)) return false;
    if (cull == CULL_NEGATIVE && area < 0.0f) return false;
    if (cull == CULL_POSITIVE && area > 0.0f) return false;

    s->minX = fmaxf(0.0f, floorf(fminf(x1, fminf(x2, x3))));
    s->maxX = fminf((float)width - 1.0f, ceilf(fmaxf(x1, fmaxf(x2, x3))));
    s->minY = fmaxf(0.0f, floorf(fminf(y1, fminf(y2, y3))));
    s->maxY = fminf((float)height - 1.0f, ceilf(fmaxf(y1, fmaxf(y2, y3))));

    if (s->minX > s->maxX || s->minY > s->maxY) return false;

    float inv = 1.0f / area;

    // barycentric A
    s->ca = ((x2 * y3 - x3 * y2) * 0.5f) * inv;
    s->dax = ((y2 - y3) * 0.5f) * inv;
    s->day = ((x3 - x2) * 0.5f) * inv;

    // barycentric B
    s->cb = ((y1 * (x3 - x1) - x1 * (y3 - y1)) * 0.5f) * inv;
    s->dbx = ((y3 - y1) * 0.5f) * inv;
    s->dby = ((x1 - x3) * 0.5f) * inv;

    // barycentric G
    s->cg = ((x1 * (y2 - y1) - y1 * (x2 - x1)) * 0.5f) * inv;
    s->dgx = ((y1 - y2) * 0.5f) * inv;
    s->dgy = ((x2 - x1) * 0.5f) * inv;

    // dx == 0 のときは使わないので inf でも構わない(行ループ側で分岐する)
    s->inv_dax = s->dax != 0.0f ? 1.0f / s->dax : 0.0f;
    s->inv_dbx = s->dbx != 0.0f ? 1.0f / s->dbx : 0.0f;
    s->inv_dgx = s->dgx != 0.0f ? 1.0f / s->dgx : 0.0f;

    s->iw1 = 1.0f / w1;
    s->iw2 = 1.0f / w2;
    s->iw3 = 1.0f / w3;

    return true;
}
