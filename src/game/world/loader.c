#include "loader.h"

#include "../render/chunk.h"
#include "chunk.h"
#include "core/raster.h"
#include "game/world/chunkmap.h"
#include "world.h"

// Static stub functions
static void generate_chunk(World *world, Chunk *chunk, ChunkPos pos) {
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

static void mesh_chunk(World *world, Chunk *chunk, ChunkPos pos) {
    if (!chunk) return;
    if (chunk->state == CHUNK_DATA_READY) {
        render_mesh_chunk(world, pos);
        chunk->state = CHUNK_READY;
    }
}

// Process queues
void process_generation_queue(ChunkLoader *loader, World *world, size_t budget) {
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

void process_mesh_queue(ChunkLoader *loader, World *world, size_t budget) {
    size_t processed = 0;
    while (processed < budget && loader->request_head < loader->requests.len) {
        ChunkPos pos = loader->requests.data[loader->request_head];
        Chunk *chunk = chunk_hash_map_get(&world->chunks, pos);
        if (chunk && chunk->state == CHUNK_DATA_READY) {
            mesh_chunk(world, chunk, pos);
        }
        loader->request_head++;
        processed++;
    }
}

void chunk_loader_update(ChunkLoader *loader, World *world, Vec3d center) {
    ChunkLoader *l = loader;
    for (int64_t z = center.z - l->rd_z; z <= center.z + l->rd_z; z++) {
        for (int64_t y = center.y - l->rd_y; y <= center.y + l->rd_y; y++) {
            for (int64_t x = center.x - l->rd_x; x <= center.x + l->rd_x; x++) {
                ChunkPos pos = {x, y, z};

                if (chunk_hash_map_get(&world->chunks, pos)) {
                    continue;
                }

                chunk_loader_request(loader, world, pos);
            }
        }
    }
}

bool chunk_loader_request(ChunkLoader *loader, World *world, ChunkPos pos) {
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

void chunk_loader_process(ChunkLoader *loader, World *world) {
    process_generation_queue(loader, world, loader->gen_budget);

    process_mesh_queue(loader, world, loader->mesh_budget);
}

ChunkLoader *chunk_loader_new(World *world) {
    (void)world;  // unused for now, but we could set default radii from world settings if needed
    ChunkLoader *loader = malloc(sizeof(*loader));
    if (!loader) return NULL;

    // Initialize vector
    loader->requests = (ChunkRequestQueue)VEC_INIT;

    loader->request_head = 0;

    loader->center.x = 0;
    loader->center.y = 0;
    loader->center.z = 0;

    loader->rd_x = 0;
    loader->rd_y = 0;
    loader->rd_z = 0;

    loader->gen_budget = 1;
    loader->mesh_budget = 1;

    return loader;
}

void chunk_loader_free(ChunkLoader *loader) {
    if (!loader) return;
    free(loader->requests.data);
    free(loader);
}

void chunk_loader_unload_distant(ChunkLoader *loader, World *world) {
    // Unload chunks that are far from the loader's center
    // Use hysteresis to avoid frequent load/unload cycles
    const int64_t hysteresis = 2;
    int64_t min_x = loader->center.x - (loader->rd_x + hysteresis);
    int64_t max_x = loader->center.x + (loader->rd_x + hysteresis);
    int64_t min_y = loader->center.y - (loader->rd_y + hysteresis);
    int64_t max_y = loader->center.y + (loader->rd_y + hysteresis);
    int64_t min_z = loader->center.z - (loader->rd_z + hysteresis);
    int64_t max_z = loader->center.z + (loader->rd_z + hysteresis);

    // Temporary list of chunks to unload
    VEC(ChunkPos) to_unload;
    to_unload.len = 0;
    to_unload.cap = 0;
    to_unload.data = NULL;

    // Iterate over all chunks in the world's hashmap
    for (size_t i = 0; i < world->chunks.cap; i++) {
        ChunkMapEntry *entry = world->chunks.entries[i];
        while (entry) {
            ChunkPos pos = entry->key;
            if (pos.x < min_x || pos.x > max_x || pos.y < min_y || pos.y > max_y || pos.z < min_z ||
                pos.z > max_z) {
                // This chunk is distant, mark for unloading
                VEC_PUSH(to_unload, pos);
            }
            entry = entry->next;
        }
    }

    // Unload all marked chunks
    for (size_t i = 0; i < to_unload.len; i++) {
        ChunkPos pos = to_unload.data[i];
        Chunk *chunk = chunk_hash_map_take(&world->chunks, pos);
        if (chunk) {
            chunk_destroy(chunk);
        }
    }

    // Free temporary vector
    free(to_unload.data);
}