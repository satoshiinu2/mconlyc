#pragma once

#include <stdint.h>

#include "core/vec.h"
#include "game/render/shader.h"

#define CHUNK_SIZE 16
#define CHUNK_DATA_SIZE CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

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
    block_t *blocks;

    VEC(TerrainVertexIn) vertices;
    VEC(int) indices;

    ChunkState state;

} Chunk;

Chunk *chunk_new();
void chunk_destroy(Chunk *chunk);

static inline int64_t floor_div16(int64_t x) {
    int64_t q = x / 16;
    int64_t r = x % 16;
    return q - (r < 0);
}

static inline int64_t floor_mod16(int64_t x) {
    int64_t r = x % 16;
    return r < 0 ? r + 16 : r;
}