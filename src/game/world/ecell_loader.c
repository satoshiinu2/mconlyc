#include "ecell_loader.h"

#include <stdint.h>
#include <stdlib.h>

#include "ecellmap.h"

static void spawn_natural_entities(World *world, EntityCell *cell, EntityCellPos pos) {
    if (cell->entities.len != 0) {
        return;  // already spawned
    }

    const size_t count = 4;
    for (size_t i = 0; i < count; i++) {
        EntityId id = ecs_create_entity(&world->ecs);
        VEC_PUSH(cell->entities, id);
    }
}

static void process_generation_queue(World *world, size_t budget) {
    EntityCellLoader *loader = world->ecell_loader;

    size_t processed = 0;

    while (processed < budget && loader->request_head < loader->requests.len) {
        EntityCellPos pos = loader->requests.data[loader->request_head];
        EntityCell *cell = entity_cell_hash_map_get(&world->spatial.cells, pos);
        if (cell) {
            spawn_natural_entities(world, cell, pos);
        }
        loader->request_head++;
        processed++;
    }
}

static bool entity_cell_loader_request_cell(World *world, EntityCellPos pos) {
    EntityCellLoader *loader = world->ecell_loader;

    // Already requested, generating, or loaded.
    if (entity_cell_hash_map_get(&world->spatial.cells, pos) != NULL) {
        return true;
    }

    EntityCell *cell = malloc_or_exit(sizeof(*cell));
    cell->entities = (EntityList)VEC_INIT(EntityId);

    // Register first to prevent duplicate requests.
    entity_cell_hash_map_insert(&world->spatial.cells, pos, cell);

    VEC_PUSH(loader->requests, pos);

    return true;
}

static void entity_cell_loader_unload_distant(World *world) {
    EntityCellLoader *loader = world->ecell_loader;

    // Unload cells that are far from the loader's center
    // Use hysteresis to avoid frequent load/unload cycles
    const int64_t hysteresis = 2;
    int64_t min_x = loader->center.x - (loader->rd_x + hysteresis);
    int64_t max_x = loader->center.x + (loader->rd_x + hysteresis);
    int64_t min_y = loader->center.y - (loader->rd_y + hysteresis);
    int64_t max_y = loader->center.y + (loader->rd_y + hysteresis);
    int64_t min_z = loader->center.z - (loader->rd_z + hysteresis);
    int64_t max_z = loader->center.z + (loader->rd_z + hysteresis);

    VEC(EntityCellPos) to_unload = VEC_INIT(EntityCellPos);

    EntityCellHashMapIterator iter = {0};
    while (entity_cell_hash_map_iter_next(&world->spatial.cells, &iter)) {
        EntityCellPos pos = iter.current->key;
        if (pos.x < min_x || pos.x > max_x || pos.y < min_y || pos.y > max_y || pos.z < min_z ||
            pos.z > max_z) {
            // This cell is distant
            VEC_PUSH(to_unload, pos);
        }
    }

    // unload all not needed
    for (size_t i = 0; i < to_unload.len; i++) {
        EntityCellPos pos = to_unload.data[i];
        EntityCell *cell = entity_cell_hash_map_take(&world->spatial.cells, pos);
        if (cell) {
            VEC_FREE(cell->entities);
            free(cell);
        }
    }

    free(to_unload.data);
}

void entity_cell_loader_update(World *world, EntityCellPos center) {
    EntityCellLoader *l = world->ecell_loader;
    l->center = center;

    for (int64_t z = center.z - l->rd_z; z <= center.z + l->rd_z; z++) {
        for (int64_t y = center.y - l->rd_y; y <= center.y + l->rd_y; y++) {
            for (int64_t x = center.x - l->rd_x; x <= center.x + l->rd_x; x++) {
                EntityCellPos pos = {x, y, z};

                if (entity_cell_hash_map_get(&world->spatial.cells, pos)) {
                    continue;
                }

                entity_cell_loader_request_cell(world, pos);
            }
        }
    }

    entity_cell_loader_unload_distant(world);
}

void entity_cell_loader_process(World *world) {
    EntityCellLoader *loader = world->ecell_loader;

    process_generation_queue(world, loader->gen_budget);
}

EntityCellLoader *entity_cell_loader_new(World *world) {
    EntityCellLoader *loader = malloc_or_exit(sizeof(*loader));

    // Initialize vector
    loader->requests = (EntityCellRequestQueue)VEC_INIT(EntityCellPos);

    loader->request_head = 0;

    loader->center.x = 0;
    loader->center.y = 0;
    loader->center.z = 0;

    loader->rd_x = 0;
    loader->rd_y = 0;
    loader->rd_z = 0;

    loader->gen_budget = 1;

    return loader;
}

void entity_cell_loader_free(EntityCellLoader *loader) {
    if (!loader) return;
    free(loader->requests.data);
    free(loader);
}

// returns added entity id
EntityId entity_cell_loader_add_entity(World *world, EntityCellPos pos, EntityId id, bool do_load) {
    EntityCellLoader *loader = world->ecell_loader;

    EntityCell *cell = entity_cell_hash_map_get(&world->spatial.cells, pos);
    if (!cell && do_load) {
        entity_cell_loader_request_cell(world, pos);
        cell = entity_cell_hash_map_get(&world->spatial.cells, pos);
    }

    if (cell) {
        if (id == EMPTY) {
            id = ecs_create_entity(&world->ecs);
        }
        VEC_PUSH(cell->entities, id);
    } else {
        return EMPTY;
    }

    return id;
}
