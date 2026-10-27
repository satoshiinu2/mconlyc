#pragma once

#include "chunkmap.h"
#include "ecellmap.h"
#include "ecs.h"

struct ChunkLoader;
struct EntityCellLoader;

typedef struct {
    ChunkHashMap chunks;
    ECS ecs;
    EntitySpatialIndex spatial;
    struct ChunkLoader *chunk_loader;
    struct EntityCellLoader *ecell_loader;
} World;

void world_init(World *world);
EntityId world_join(World *world, Vec3d player_pos);
void world_update(World *world);
void world_destroy(World *world);
