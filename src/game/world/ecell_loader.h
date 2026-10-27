#pragma once

#include "ecellmap.h"
#include "world.h"

typedef VEC(EntityCellPos) EntityCellRequestQueue;

typedef struct EntityCellLoader {
    EntityCellRequestQueue requests;
    size_t request_head;

    EntityCellPos center;
    int rd_x;
    int rd_y;
    int rd_z;

    size_t gen_budget;
} EntityCellLoader;

EntityCellLoader *entity_cell_loader_new(World *world);

void entity_cell_loader_update(World *world, EntityCellPos center);

void entity_cell_loader_process(World *world);

void entity_cell_loader_free(EntityCellLoader *loader);

EntityId entity_cell_loader_add_entity(World *world, EntityCellPos pos, EntityId id, bool do_load);
