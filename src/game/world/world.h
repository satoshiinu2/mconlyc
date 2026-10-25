#pragma once

#include "chunkmap.h"

typedef struct ChunkLoader ChunkLoader;

typedef struct {
    ChunkHashMap chunks;
    ChunkLoader *loader;
} World;

void world_init(World *world);
void world_destroy(World *world);