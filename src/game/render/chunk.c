#include "chunk.h"

#include <stddef.h>
#include <stdint.h>

#include "core/window.h"
#include "game/state.h"
#include "game/world/blocks.h"
#include "game/world/chunk.h"
#include "game/world/chunkmap.h"
#include "game/world/world.h"
#include "render.h"
#include "shader.h"

void render_mesh_chunk(World *world, ChunkPos pos) {
    ChunkCache cache = gen_chunkcache(world, pos.x - 1, pos.y - 1, pos.z - 1, 3, 3, 3);
    Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);

    VEC_CLEAR(chunk->vertices);
    VEC_CLEAR(chunk->indices);

    for (size_t y = 0; y < CHUNK_SIZE; y++) {
        for (size_t z = 0; z < CHUNK_SIZE; z++) {
            for (size_t x = 0; x < CHUNK_SIZE; x++) {
            }
        }
    }

    free(cache.chunks);
}

void render_chunk(Window *window, Chunk *chunk) {
    uint32_t width = window->render_mgr.width;
    uint32_t height = window->render_mgr.height;

    Mat4 model = mat4_identity();
    Mat4 view = mat4_translate(0, 0, -5);
    Mat4 proj = mat4_perspective(1.0f, (float)width / height, NEAR_VAL, 100.0f);
    Mat4 vp = mat4_viewport(width, height);
    Mat4 mvp = mat4_mul(vp, mat4_mul(proj, mat4_mul(view, model)));

    GameState *state = window->state;

    TerrainUniforms uniforms = {.texture = &state->terrain_texture, .mvp = &mvp};

    TerrainVertexIn *vertices = chunk->vertices.data;
    int *indices = chunk->indices.data;
    size_t index_count = chunk->indices.len;

    render_elements_terrain(&window->render_mgr, &uniforms, NEAR_VAL, vertices, indices,
                            index_count);
}

ChunkCache gen_chunkcache(World *world, int64_t x_start, int64_t y_start, int64_t z_start,
                          int64_t width, int64_t height, int64_t depth) {
    assert(world != NULL);
    assert(width > 0 && height > 0 && depth > 0);

    // Validate multiplication before allocating.
    size_t w = (size_t)width;
    size_t h = (size_t)height;
    size_t d = (size_t)depth;

    assert(w <= SIZE_MAX / h);
    size_t wh = w * h;

    assert(wh <= SIZE_MAX / d);
    size_t count = wh * d;

    assert(count <= SIZE_MAX / sizeof(Chunk *));

    Chunk **chunks = malloc_or_exit(count * sizeof(*chunks));

    for (int64_t y = 0; y < height; y++) {
    for (int64_t z = 0; z < depth; z++) {
            for (int64_t x = 0; x < width; x++) {
                ChunkPos pos = {
                    .x = (int32_t)(x_start + x),
                    .y = (int32_t)(y_start + y),
                    .z = (int32_t)(z_start + z),
                };

                size_t index = chunkcache_index(x, y, z, width, depth);

                chunks[index] = chunk_hash_map_get(&world->chunks, pos);
            }
        }
    }

    return (ChunkCache){
        .chunk_x = x_start,
        .chunk_y = y_start,
        .chunk_z = z_start,
        .width = width,
        .height = height,
        .depth = depth,
        .chunks = chunks,
    };
}

block_t chunkcache_get_block(ChunkCache *cache, int64_t x, int64_t y, int64_t z) {
    int64_t chunk_x = floor_div16(x);
    int64_t chunk_y = floor_div16(y);
    int64_t chunk_z = floor_div16(z);

    int64_t in_x = floor_mod16(x);
    int64_t in_y = floor_mod16(y);
    int64_t in_z = floor_mod16(z);

    size_t index_x = chunk_x - cache->chunk_x;
    size_t index_y = chunk_y - cache->chunk_x;
    size_t index_z = chunk_z - cache->chunk_x;

    if (index_x < 0 || index_x >= cache->width || index_y < 0 || index_y >= cache->height ||
        index_z < 0 || index_z >= cache->depth) {
        return BLOCK_AIR;
    }

    size_t t_index = chunkcache_index(index_x, index_y, index_z, cache->width, cache->depth);

    Chunk *chunk = cache->chunks[t_index];

    size_t b_index = chunk_index(in_x, in_y, in_z);

    block_t block = chunk->blocks[b_index];

    return block;
}