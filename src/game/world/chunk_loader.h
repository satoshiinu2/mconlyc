#pragma once

#include "chunkmap.h"
#include "core/raster.h"
#include "world.h"

typedef VEC(ChunkPos) ChunkRequestQueue;

typedef struct ChunkLoader {
    ChunkRequestQueue requests;
    size_t request_head;
    size_t mesh_head;

    ChunkPos center;
    int rd_x;
    int rd_y;
    int rd_z;

    size_t gen_budget;
    size_t mesh_budget;
} ChunkLoader;

ChunkLoader *chunk_loader_new(World *world);

void chunk_loader_update(World *world, ChunkPos center);

void chunk_loader_process(World *world);

void chunk_loader_free(ChunkLoader *loader);
