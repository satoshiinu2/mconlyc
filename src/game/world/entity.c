#include "ecell_loader.h"
#include "ecellmap.h"
#include "ecs.h"
#include "world.h"

static void update_entities_cell(World *world) {
    typedef struct {
        EntityId id;
        EntityCellPos source;
        EntityCellPos destination;
    } CellUpdate;
    VEC(CellUpdate) updates = VEC_INIT(CellUpdate);
    EntityCellHashMapIterator cell_iter = {0};
    EntityCell *cell;

    while ((cell = entity_cell_hash_map_iter_next(&world->spatial.cells, &cell_iter))) {
        EntityCellPos cell_pos = cell_iter.current->key;

        EcsIterator entity_iter = {.required = ECS_COM_POSITION};

        EntityId entity_id;
        while ((entity_id = entity_cell_iter_next(cell, &world->ecs, &entity_iter))) {
            Vec3d *entity_pos = ECS_GET(&world->ecs, entity_id, position);

            EntityCellPos new_cell = world_pos_to_entity_cell(*entity_pos);

            VEC_PUSH(updates, ((CellUpdate){entity_id, cell_pos, new_cell}));
        }
    }

    // Finish both iterators before inserting, rehashing, or unloading cells.
    for (size_t i = 0; i < updates.len; i++) {
        CellUpdate update = updates.data[i];
        if (update.source.x == update.destination.x && update.source.y == update.destination.y &&
            update.source.z == update.destination.z)
            continue;

        entity_cell_loader_add_entity(world, update.destination, update.id, true);
        EntityCell *source = entity_cell_hash_map_get(&world->spatial.cells, update.source);
        for (size_t j = 0; j < source->entities.len; j++) {
            if (source->entities.data[j] == update.id) {
                VEC_SWAP_REMOVE(source->entities, j);
                break;
            }
        }
    }

    for (size_t i = 0; i < updates.len; i++) {
        entity_cell_loader_update(world, updates.data[i].destination);
    }
    VEC_FREE(updates);
}

static void update_entities_input(World *world) {
    EcsIterator iter = {.required = ECS_COM_VELOCITY | ECS_COM_INPUT_VEC};
    EntityId entity_id;

    while ((entity_id = ecs_iter_next(&world->ecs, &iter))) {
        Vec3 *veclocity = ECS_GET(&world->ecs, entity_id, velocity);
        Vec2 *input_vec = ECS_GET(&world->ecs, entity_id, input_vector);

        Vec3 accelerator = {.x = input_vec->x, .z = input_vec->y};

        veclocity->x += accelerator.x;
        veclocity->y += accelerator.y;
        veclocity->z += accelerator.z;
    }
}

static void update_entities_phy(World *world) {
    EcsIterator iter = {.required = ECS_COM_POSITION | ECS_COM_VELOCITY};
    EntityId entity_id;

    while ((entity_id = ecs_iter_next(&world->ecs, &iter))) {
        Vec3d *position = ECS_GET(&world->ecs, entity_id, position);
        Vec3 *veclocity = ECS_GET(&world->ecs, entity_id, velocity);

        position->x += veclocity->x;
        position->y += veclocity->y;
        position->z += veclocity->z;

        const float air_resistance = 0.8;

        veclocity->x *= air_resistance;
        veclocity->y *= air_resistance;
        veclocity->z *= air_resistance;
    }
}

void update_entities(World *world) {
    update_entities_input(world);
    update_entities_phy(world);
    update_entities_cell(world);
}
