#pragma once

#include <stdint.h>

typedef uint32_t pixel_t;
typedef void *uniform_t;
typedef void *vertex_in_t;
typedef void *vertex_out_t;

#define WINDOW_TITLE "Minecraft on C without SDL"
#define WINDOW_WIDTH 854
#define WINDOW_HEIGHT 480

typedef struct {
    int x;
    int y;
} UiVec2;

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

Vec3 project(Vec3 a);
Vec4 view_to_clip(Vec3 a, float focal, float width, float height);
Vec3 rotate_x3(Vec3 a, float t);
Vec3 rotate_y3(Vec3 a, float t);
Vec3 rotate_z3(Vec3 a, float t);
Vec3 translate_x3(Vec3 a, float x);
Vec3 translate_y3(Vec3 a, float y);
Vec3 translate_z3(Vec3 a, float z);
