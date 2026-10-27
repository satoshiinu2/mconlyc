#include "chunk_gen.h"

#include "chunk.h"

void generate_chunk(World *world, Chunk *chunk, ChunkPos pos) {
    if (!chunk) return;
    if (chunk->state != CHUNK_REQUESTED) return;

    const int64_t surface_y = 4;  // flat surface height (world Y)
    const int64_t base_y = pos.y * CHUNK_SIZE;

    for (int64_t y = 0; y < CHUNK_SIZE; y++) {
        const int64_t world_y = base_y + y;
        for (int64_t z = 0; z < CHUNK_SIZE; z++) {
            for (int64_t x = 0; x < CHUNK_SIZE; x++) {
                const size_t index = chunk_index(x, y, z);
                if (world_y < surface_y) {
                    chunk->blocks[index] = BLOCK_STONE;
                } else if (world_y == surface_y) {
                    chunk->blocks[index] = BLOCK_GRASS;
                } else {
                    chunk->blocks[index] = BLOCK_AIR;
                }
            }
        }
    }

    chunk->state = CHUNK_DATA_READY;
}