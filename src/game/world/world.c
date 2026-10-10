#include "world.h"

#include "chunkmap.h"

void world_init(World *world) {
    world->chunks = chunk_hash_map_new(5);

}