#pragma  once

#include <stdint.h>

#include "core/vec.h"
#include "render/shader.h"

#define CHUNK_SIZE 16

typedef struct {
    uint8_t blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    VEC(TerrainVertexIn) vertices;
    VEC(int) indices;
} Chunk;