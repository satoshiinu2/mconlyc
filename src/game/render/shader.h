#pragma once

#include "core/bmp.h"
#include "core/math.h"
#include "core/render.h"

typedef struct {
    Vec3 position;
} BasicVertexIn;

typedef struct {
    Vec4 clip_pos;
} BasicVertexOut;

typedef struct {
    float fov;
    float width;
    float height;
    float z_offset;
    float time;
} BasicUniforms;
typedef struct {
    Vec3 position;
    Vec2 uv;
} TerrainVertexIn;

typedef struct {
    Vec4 clip_pos;
    Vec2 uv;
} TerrainVertexOut;

typedef struct {
    const Texture *texture;
    const Mat4 *mvp;
} TerrainUniforms;

Shader create_basic_shader();

Shader create_terrain_shader();