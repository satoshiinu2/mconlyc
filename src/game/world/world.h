#pragma once

#include "chunkmap.h"

typedef struct {
    ChunkHashMap chunks;
} World;

void world_init(World*world);