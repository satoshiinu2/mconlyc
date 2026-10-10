#include "chunk.h"

#include <stddef.h>

#include "core/window.h"
#include "game/chunk.h"
#include "game/state.h"
#include "game/world.h"
#include "render.h"
#include "shader.h"

void rebake_render_chunk_mesh(World *world, Chunk *chunk) {
    VEC_CLEAR(chunk->vertices);
    VEC_CLEAR(chunk->indices);
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
    Chunk **chunks = malloc_or_exit((width + 1) * (height + 1) * (depth + 1) * sizeof(Chunk *));

    for (int64_t x = x_start; x < x + width; x++) {
        for (int64_t y = y_start; y < y + width; y++) {
            for (int64_t z = z_start; z < z + width; z++) {
                ChunkPos pos = {x, y, z};
                Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);
                size_t index = chunkcache_index(x, y, z, width, height);

                chunks[index] = chunk;
            }
        }
    }

    return (ChunkCache){
        .chunks = chunks,
    };
}