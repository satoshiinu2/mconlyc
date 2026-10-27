

#include <stddef.h>
#include <stdint.h>

#include "game/world/world.h"

// must free chunks
typedef struct {
    Chunk **chunks;
    int64_t chunk_x, chunk_y, chunk_z;
    int64_t width, height, depth;
} ChunkCache;

// 所有権返す
ChunkCache gen_chunkcache(World *world, int64_t x_start, int64_t y_start, int64_t z_start,
                          int64_t width, int64_t height, int64_t depth);

// y z x
static inline size_t chunkcache_index(int64_t x, int64_t y, int64_t z, int64_t width,
                                      int64_t depth) {
    return (size_t)x + (size_t)width * ((size_t)z + (size_t)depth * (size_t)y);
}



void render_mesh_chunk(World *world, ChunkPos pos);
Blocks chunkcache_get_block(ChunkCache *cache, int64_t x, int64_t y, int64_t z);