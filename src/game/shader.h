#pragma once

#include "core/render.h"

// Define shader types for basic shader
typedef struct
{
    Vec3 position;
} BasicVertexIn;

typedef struct
{
    Vec4 clip_pos;
} BasicVertexOut;

typedef struct
{
    float scale;
    float width;
    float height;
    float z_offset;
    float time;
} BasicUniforms;

Shader create_basic_shader();