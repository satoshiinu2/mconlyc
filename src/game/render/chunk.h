

#include <stddef.h>
#include <stdint.h>

#include "game/world.h"

// must free chunks
typedef struct {
    Chunk **chunks;
} ChunkCache;

// 所有権返す
ChunkCache gen_chunkcache(World *world, int64_t x_start, int64_t y_start, int64_t z_start,
                          int64_t width, int64_t height, int64_t depth);

// x z y
static inline size_t chunkcache_index(int64_t x, int64_t y, int64_t z, int64_t width,
                                      int64_t height) {
    return y * height * width + x * width + z;
}