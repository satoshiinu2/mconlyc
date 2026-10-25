#include "world.h"

#include "chunkmap.h"
#include "loader.h"

void world_init(World *world) {
    world->chunks = chunk_hash_map_new(5);
    world->loader = chunk_loader_new(world);
}

void world_destroy(World *world) {
    if (!world) {
        return;
    }

    if (world->loader) {
        chunk_loader_free(world->loader);
        world->loader = NULL;
    }

    chunk_hash_map_free(&world->chunks);
}