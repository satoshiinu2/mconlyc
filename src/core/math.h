#pragma once

#include <float.h>
#include <math.h>

#include "min.h"

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

typedef struct {
    float m[16];  // 列優先: m[col * 4 + row]
} Mat4;

static inline Vec4 rgba8_to_vec4(const pixel_t p) {
    return (Vec4){
        (p & 255) / 255.0f,
        ((p >> 8) & 255) / 255.0f,
        ((p >> 16) & 255) / 255.0f,
        ((p >> 24) & 255) / 255.0f,
    };
}

static inline Vec3 vec3_add(const Vec3 a, const Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 vec3_sub(const Vec3 a, const Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline float vec3_dot(const Vec3 a, const Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline Vec3 vec3_cross(const Vec3 a, const Vec3 b) {
    return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

static inline Vec3 vec3_norm(const Vec3 v) {
    const float len = sqrtf(vec3_dot(v, v));
    if (len == 0) return v;
    return (Vec3){v.x / len, v.y / len, v.z / len};
}

static inline Vec3 vec3_rotate_x(const Vec3 a, const float t) {
    const float c = cos(t), s = sin(t);
    return (Vec3){a.x, a.y * c - a.z * s, a.y * s + a.z * c};
}

static inline Vec3 vec3_rotate_y(const Vec3 a, const float t) {
    const float c = cos(t), s = sin(t);
    return (Vec3){a.x * c + a.z * s, a.y, -a.x * s + a.z * c};
}

static inline Vec3 vec3_rotate_z(const Vec3 a, const float t) {
    const float c = cos(t), s = sin(t);
    return (Vec3){a.x * c - a.y * s, a.x * s + a.y * c, a.z};
}

static inline Vec3 vec3_translate_x(const Vec3 a, const float x) {
    return (Vec3){a.x + x, a.y, a.z};
}

static inline Vec3 vec3_translate_y(const Vec3 a, const float y) {
    return (Vec3){a.x, a.y + y, a.z};
}

static inline Vec3 vec3_translate_z(const Vec3 a, const float z) {
    return (Vec3){a.x, a.y, a.z + z};
}

static inline Mat4 mat4_identity(void) {
    Mat4 r = {{0}};
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1;
    return r;
}

static inline Mat4 mat4_mul(const Mat4 a, const Mat4 b) {
    Mat4 r;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float sum = 0;
            for (int k = 0; k < 4; k++) {
                sum += a.m[k * 4 + row] * b.m[col * 4 + k];
            }
            r.m[col * 4 + row] = sum;
        }
    }
    return r;
}

static inline Vec4 mat4_mul_vec4(const Mat4 *m, const Vec4 v) {
    return (Vec4){
        m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z + m->m[12] * v.w,
        m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z + m->m[13] * v.w,
        m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z + m->m[14] * v.w,
        m->m[3] * v.x + m->m[7] * v.y + m->m[11] * v.z + m->m[15] * v.w,
    };
}

static inline Vec3 mat4_mul_point(const Mat4 *m, const Vec3 v) {
    return (Vec3){
        m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z + m->m[12],
        m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z + m->m[13],
        m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z + m->m[14],
    };
}

static inline Vec3 mat4_mul_dir(const Mat4 *m, const Vec3 v) {
    return (Vec3){
        m->m[0] * v.x + m->m[4] * v.y + m->m[8] * v.z,
        m->m[1] * v.x + m->m[5] * v.y + m->m[9] * v.z,
        m->m[2] * v.x + m->m[6] * v.y + m->m[10] * v.z,
    };
}

static inline Mat4 mat4_transpose(const Mat4 m) {
    Mat4 r;
    for (int col = 0; col < 4; col++)
        for (int row = 0; row < 4; row++) r.m[col * 4 + row] = m.m[row * 4 + col];
    return r;
}

static inline Mat4 mat4_translate(const float x, const float y, const float z) {
    Mat4 r = mat4_identity();
    r.m[12] = x;
    r.m[13] = y;
    r.m[14] = z;
    return r;
}

static inline Mat4 mat4_scale(const float x, const float y, const float z) {
    Mat4 r = mat4_identity();
    r.m[0] = x;
    r.m[5] = y;
    r.m[10] = z;
    return r;
}

static inline Mat4 mat4_rotate_x(const float rad) {
    const float c = cosf(rad), s = sinf(rad);
    Mat4 r = mat4_identity();
    r.m[5] = c;
    r.m[6] = s;
    r.m[9] = -s;
    r.m[10] = c;
    return r;
}

static inline Mat4 mat4_rotate_y(const float rad) {
    const float c = cosf(rad), s = sinf(rad);
    Mat4 r = mat4_identity();
    r.m[0] = c;
    r.m[2] = -s;
    r.m[8] = s;
    r.m[10] = c;
    return r;
}

static inline Mat4 mat4_rotate_z(const float rad) {
    const float c = cosf(rad), s = sinf(rad);
    Mat4 r = mat4_identity();
    r.m[0] = c;
    r.m[1] = s;
    r.m[4] = -s;
    r.m[5] = c;
    return r;
}

// ビュー行列: ワールド → カメラ空間 (カメラは -Z を向く)
static inline Mat4 mat4_look_at(const Vec3 eye, const Vec3 center, const Vec3 up) {
    const Vec3 f = vec3_norm(vec3_sub(center, eye));
    const Vec3 s = vec3_norm(vec3_cross(f, up));
    const Vec3 u = vec3_cross(s, f);

    Mat4 r = mat4_identity();
    r.m[0] = s.x;
    r.m[4] = s.y;
    r.m[8] = s.z;
    r.m[1] = u.x;
    r.m[5] = u.y;
    r.m[9] = u.z;
    r.m[2] = -f.x;
    r.m[6] = -f.y;
    r.m[10] = -f.z;
    r.m[12] = -vec3_dot(s, eye);
    r.m[13] = -vec3_dot(u, eye);
    r.m[14] = vec3_dot(f, eye);
    return r;
}

// 射影行列: カメラ空間 → クリップ空間 (w = -z_view)
// fov_y はラジアン、aspect = width / height
static inline Mat4 mat4_perspective(const float fov_y, const float aspect, const float near_plane,
                                    const float far_plane) {
    const float f = 1.0f / tanf(fov_y * 0.5f);
    Mat4 r = {{0}};
    r.m[0] = f / aspect;
    r.m[5] = f;
    r.m[10] = (far_plane + near_plane) / (near_plane - far_plane);
    r.m[11] = -1;
    r.m[14] = (2 * far_plane * near_plane) / (near_plane - far_plane);
    return r;
}

// クリップ空間 → ピクセル座標 (w はそのまま保持、Y 反転)
// この規約だと x / w, y / w がそのままピクセル座標になる
static inline Mat4 mat4_viewport(const float width, const float height) {
    Mat4 r = mat4_identity();
    r.m[0] = width * 0.5f;
    r.m[12] = width * 0.5f;  // x' = w/2 * x + w/2 * clip.w
    r.m[5] = -height * 0.5f;
    r.m[13] = height * 0.5f;  // y' = -h/2 * y + h/2 * clip.w
    return r;
}