#include "chunk_loader.h"

#include "../render/chunk.h"
#include "chunk.h"
#include "chunk_gen.h"
#include "game/world/chunkmap.h"
#include "world.h"

static void try_mesh_chunk(World *world, Chunk *chunk, ChunkPos pos) {
    if (!chunk) return;
    if (chunk->state == CHUNK_DATA_READY) {
        render_mesh_chunk(world, pos);
        chunk->state = CHUNK_READY;
    }
}

static void process_generation_queue(World *world, size_t budget) {
    ChunkLoader *loader = world->chunk_loader;

    size_t processed = 0;
    while (processed < budget && loader->request_head < loader->requests.len) {
        ChunkPos pos = loader->requests.data[loader->request_head];
        Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);
        if (chunk && chunk->state == CHUNK_REQUESTED) {
            generate_chunk(world, chunk, pos);
        }
        loader->request_head++;
        processed++;
    }
}

static void process_mesh_queue(World *world, size_t budget) {
    ChunkLoader *loader = world->chunk_loader;

    size_t processed = 0;
    while (processed < budget && loader->mesh_head < loader->request_head) {
        ChunkPos pos = loader->requests.data[loader->mesh_head];
        Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);
        if (chunk && chunk->state == CHUNK_DATA_READY) {
            try_mesh_chunk(world, chunk, pos);
        }
        loader->mesh_head++;
        processed++;
    }
}

static bool chunk_loader_request_chunk(World *world, ChunkPos pos) {
    ChunkLoader *loader = world->chunk_loader;

    // Already requested, generating, or loaded.
    if (chunk_hash_map_get(&world->chunks, pos) != NULL) {
        return true;
    }

    Chunk *chunk = chunk_new();

    chunk->state = CHUNK_REQUESTED;

    // Register first to prevent duplicate requests.
    chunk_hash_map_insert(&world->chunks, pos, chunk);

    VEC_PUSH(loader->requests, pos);

    return true;
}

static void chunk_loader_unload_distant(World *world) {
    ChunkLoader *loader = world->chunk_loader;

    // Unload chunks that are far from the loader's center
    // Use hysteresis to avoid frequent load/unload cycles
    const int64_t hysteresis = 2;
    int64_t min_x = loader->center.x - (loader->rd_x + hysteresis);
    int64_t max_x = loader->center.x + (loader->rd_x + hysteresis);
    int64_t min_y = loader->center.y - (loader->rd_y + hysteresis);
    int64_t max_y = loader->center.y + (loader->rd_y + hysteresis);
    int64_t min_z = loader->center.z - (loader->rd_z + hysteresis);
    int64_t max_z = loader->center.z + (loader->rd_z + hysteresis);

    VEC(ChunkPos) to_unload = VEC_INIT(ChunkPos);

    ChunkHashMapIterator iter = {0};
    while (chunk_hash_map_iter_next(&world->chunks, &iter)) {
        ChunkPos pos = iter.current->key;
        if (pos.x < min_x || pos.x > max_x || pos.y < min_y || pos.y > max_y || pos.z < min_z ||
            pos.z > max_z) {
            // This chunk is distant
            VEC_PUSH(to_unload, pos);
        }
    }

    // unload all not needed
    for (size_t i = 0; i < to_unload.len; i++) {
        ChunkPos pos = to_unload.data[i];
        Chunk *chunk = chunk_hash_map_take(&world->chunks, pos);
        if (chunk) {
            chunk_destroy(chunk);
        }
    }

    free(to_unload.data);
}

void chunk_loader_update(World *world, ChunkPos center) {
    ChunkLoader *l = world->chunk_loader;
    l->center = center;

    for (int64_t z = center.z - l->rd_z; z <= center.z + l->rd_z; z++) {
        for (int64_t y = center.y - l->rd_y; y <= center.y + l->rd_y; y++) {
            for (int64_t x = center.x - l->rd_x; x <= center.x + l->rd_x; x++) {
                ChunkPos pos = {x, y, z};

                if (chunk_hash_map_get(&world->chunks, pos)) {
                    continue;
                }

                chunk_loader_request_chunk(world, pos);
            }
        }
    }

    chunk_loader_unload_distant(world);
}

void chunk_loader_process(World *world) {
    ChunkLoader *loader = world->chunk_loader;

    process_generation_queue(world, loader->gen_budget);

    process_mesh_queue(world, loader->mesh_budget);
}

ChunkLoader *chunk_loader_new(World *world) {
    ChunkLoader *loader = malloc_or_exit(sizeof(*loader));

    // Initialize vector
    loader->requests = (ChunkRequestQueue)VEC_INIT(ChunkPos);

    loader->request_head = 0;
    loader->mesh_head = 0;

    loader->center.x = 0;
    loader->center.y = 0;
    loader->center.z = 0;

    loader->rd_x = 1;
    loader->rd_y = 0;
    loader->rd_z = 1;

    loader->gen_budget = 1;
    loader->mesh_budget = 1;

    return loader;
}

void chunk_loader_free(ChunkLoader *loader) {
    if (!loader) return;
    free(loader->requests.data);
    free(loader);
}
