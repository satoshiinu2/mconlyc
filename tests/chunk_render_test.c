#include <assert.h>

#include "core/window.h"
#include "game/render/chunk.h"
#include "game/render/render.h"
#include "game/state.h"
#include "game/world/chunk_loader.h"

int main(void) {
    World world = {0};
    world_init(&world);
    ChunkPos center = {0};
    chunk_loader_update(&world, center);
    assert(world.chunks.count == 9);
    world.chunk_loader->gen_budget = 2;
    world.chunk_loader->mesh_budget = 0;
    chunk_loader_process(&world);
    assert(world.chunk_loader->request_head == 2);
    assert(world.chunk_loader->mesh_head == 0);
    world.chunk_loader->mesh_budget = 1;
    for (size_t i = 0; i < 12; i++) chunk_loader_process(&world);
    ChunkHashMapIterator iter = {0};
    Chunk *chunk;
    while ((chunk = chunk_hash_map_iter_next(&world.chunks, &iter))) {
        assert(chunk->state == CHUNK_READY);
        assert(chunk->vertices.len > 0 && chunk->indices.len > 0);
    }

    // Exercise the real renderer without opening a window.
    pixel_t white = 0xffffffff;
    GameState state = {.world = &world,
                       .terrain_texture = {.width = 1, .height = 1, .pixels = &white},
                       .camera = {.position = {8, 8, 8}}};
    Window window = {.state = &state};
    window.render_mgr.width = 160;
    window.render_mgr.height = 120;
    size_t pixels = 160 * 120;
    window.render_mgr.pixel_buffer = calloc_or_exit(pixels, sizeof(pixel_t));
    window.render_mgr.depth_buffer = malloc_or_exit(pixels * sizeof(float));
    clear_render_buffers(&window.render_mgr);
    render_chunks(&window);
    size_t drawn = 0;
    for (size_t i = 0; i < pixels; i++) drawn += window.render_mgr.depth_buffer[i] > 0;
    assert(drawn > 100);
    free(window.render_mgr.pixel_buffer);
    free(window.render_mgr.depth_buffer);

    // Unequal cache origins, missing neighbors, and a single block's six faces.
    ChunkPos remote = {7, -2, -5};
    Chunk *single = chunk_new();
    single->state = CHUNK_DATA_READY;
    single->blocks[chunk_index(2, 3, 4)] = BLOCK_STONE;
    chunk_hash_map_insert(&world.chunks, remote, single);
    ChunkCache cache = gen_chunkcache(&world, 6, -3, -6, 3, 3, 3);
    assert(chunkcache_get_block(&cache, 7 * CHUNK_SIZE + 2,
                               -2 * CHUNK_SIZE + 3, -5 * CHUNK_SIZE + 4) == BLOCK_STONE);
    assert(chunkcache_get_block(&cache, 6 * CHUNK_SIZE, -3 * CHUNK_SIZE,
                               -6 * CHUNK_SIZE) == BLOCK_AIR);
    free(cache.chunks);
    render_mesh_chunk(&world, remote);
    assert(single->vertices.len == 24 && single->indices.len == 36);

    center = (ChunkPos){10, 0, -10};
    chunk_loader_update(&world, center);
    assert(world.chunk_loader->center.x == 10 && world.chunk_loader->center.z == -10);
    assert(world.chunks.count == 9);
    for (size_t i = 0; i < 12; i++) chunk_loader_process(&world);
    assert(chunk_hash_map_get(&world.chunks, center)->state == CHUNK_READY);
    world_destroy(&world);
    return 0;
}
