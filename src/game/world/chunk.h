#pragma once

#include <stdint.h>

#include "core/vec.h"
#include "game/render/shader.h"
#include "game/world/blocks.h"

#define CHUNK_SIZE 16
#define CHUNK_DATA_SIZE CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

// y z x
static inline size_t chunk_index(int64_t x, int64_t y, int64_t z) {
    return (size_t)x + (size_t)CHUNK_SIZE * ((size_t)y + (size_t)CHUNK_SIZE * (size_t)z);
}
typedef enum {
    CHUNK_REQUESTED,
    CHUNK_GENERATING,
    CHUNK_DATA_READY,
    CHUNK_MESHING,
    CHUNK_MESH_READY,
    CHUNK_READY,
    CHUNK_FAILED,
} ChunkState;

typedef struct {
    Blocks *blocks;

    VEC(TerrainVertexIn) vertices;
    VEC(int) indices;

    ChunkState state;

} Chunk;

Chunk *chunk_new();
void chunk_destroy(Chunk *chunk);
