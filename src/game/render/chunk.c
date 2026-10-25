#include "chunk.h"

#include <stddef.h>
#include <stdint.h>

#include "core/math.h"
#include "core/window.h"
#include "game/state.h"
#include "game/world/blocks.h"
#include "game/world/chunk.h"
#include "game/world/chunkmap.h"
#include "game/world/world.h"
#include "render.h"
#include "shader.h"

#define ATLAS_SIZE 16.0f
typedef struct {
    // 隣接方向
    int dx, dy, dz;
    // 4頂点 (外から見て CCW)
    Vec3 v[4];
    float shade;
} FaceDef;

static const FaceDef FACES[6] = {
    // +X
    {+1, 0, 0, {{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}, 0.8f},
    // -X
    {-1, 0, 0, {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}}, 0.8f},
    // +Y (上)
    {0, +1, 0, {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}}, 1.0f},
    // -Y (下)
    {0, -1, 0, {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}, 0.5f},
    // +Z
    {0, 0, +1, {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}, 0.9f},
    // -Z
    {0, 0, -1, {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}}, 0.9f},
};

// 4頂点に対応する UV (タイル内 0..1)
static const float FACE_UV[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};

static void push_face(Chunk *chunk, int x, int y, int z, const FaceDef *f, float tile_u,
                      float tile_v, float tile_size, Vec3 color) {
    uint32_t base = (uint32_t)chunk->vertices.len;

    for (int i = 0; i < 4; i++) {
        TerrainVertexIn vtx = {
            .position = {x + f->v[i].x, y + f->v[i].y, z + f->v[i].z},
            .uv = {tile_u + FACE_UV[i][0] * tile_size, tile_v + FACE_UV[i][1] * tile_size},
            .color = {f->shade * color.x, f->shade * color.y, f->shade * color.z},
        };
        VEC_PUSH(chunk->vertices, vtx);
    }

    int idx[6] = {base, base + 1, base + 2, base, base + 2, base + 3};
    for (int i = 0; i < 6; i++) {
        VEC_PUSH(chunk->indices, idx[i]);
    }
}

void render_mesh_chunk(World *world, ChunkPos pos) {
    ChunkCache cache = gen_chunkcache(world, pos.x - 1, pos.y - 1, pos.z - 1, 3, 3, 3);
    Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);

    VEC_CLEAR(chunk->vertices);
    VEC_CLEAR(chunk->indices);

    for (size_t y = 0; y < CHUNK_SIZE; y++) {
        for (size_t z = 0; z < CHUNK_SIZE; z++) {
            for (size_t x = 0; x < CHUNK_SIZE; x++) {
                for (Faces face = 0; face < FACE_COUNT; face++) {
                    const FaceDef *f = &FACES[face];

                    Blocks nb = chunkcache_get_block(&cache, (int64_t)pos.x * CHUNK_SIZE + x,
                                                     (int64_t)pos.y * CHUNK_SIZE + y,
                                                     (int64_t)pos.z * CHUNK_SIZE + z);

                    if (BLOCK_DEFS[nb].opaque) continue;  // 　埋まっている

                    float tile_u = 0.0f, tile_v = 0.0f, tile_size = 1.0f / ATLAS_SIZE;

                    Vec3 color = {1.0f, 1.0f, 1.0f};
                    push_face(chunk, x, y, z, f, tile_u, tile_v, tile_size, color);
                }
            }
        }

        free(cache.chunks);
    }
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

Blocks chunkcache_get_block(ChunkCache *cache, int64_t x, int64_t y, int64_t z) {
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

    Blocks block = chunk->blocks[b_index];

    return block;
}