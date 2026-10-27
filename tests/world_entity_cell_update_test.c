#include <assert.h>

#include "game/world/ecell_loader.h"
#include "game/world/world.h"

static size_t count_entity(World *world, EntityId id) {
    size_t count = 0;
    EntityCellHashMapIterator iter = {0};
    EntityCell *cell;
    while ((cell = entity_cell_hash_map_iter_next(&world->spatial.cells, &iter))) {
        for (size_t i = 0; i < cell->entities.len; i++) {
            count += cell->entities.data[i] == id;
        }
    }
    return count;
}

int main(void) {
    World world = {0};
    ecs_init(&world.ecs);
    world.spatial.cells = entity_cell_hash_map_new(1);
    world.ecell_loader = entity_cell_loader_new(&world);
    EntityId ids[3];
    for (size_t i = 0; i < 3; i++) {
        ids[i] = ecs_create_entity(&world.ecs);
        ECS_SET(&world.ecs, ids[i], position, ((Vec3d){0}));
        entity_cell_loader_add_entity(&world, (EntityCellPos){0}, ids[i], true);
    }

    // Move two entities out of the same list, forcing insertion and rehashing.
    for (size_t i = 0; i < 2; i++) {
        ECS_SET(&world.ecs, ids[i], position, ((Vec3d){ENTITY_CELL_SIZE, 0, 0}));
    }
    world_update(&world);
    EntityCell *origin = entity_cell_hash_map_get(&world.spatial.cells, (EntityCellPos){0});
    assert(origin && origin->entities.len == 1 && origin->entities.data[0] == ids[2]);
    for (size_t i = 0; i < 3; i++) assert(count_entity(&world, ids[i]) == 1);

    // Teleport beyond hysteresis: the old cells are freed during loader update.
    // Repeating also verifies that destination IDs do not accumulate duplicates.
    for (size_t frame = 1; frame <= 20; frame++) {
        Vec3d pos = {frame * 10 * ENTITY_CELL_SIZE, 0, 0};
        for (size_t i = 0; i < 3; i++) ECS_SET(&world.ecs, ids[i], position, pos);
        world_update(&world);
        EntityCell *cell = entity_cell_hash_map_get(&world.spatial.cells,
                                                    world_pos_to_entity_cell(pos));
        assert(cell && cell->entities.len == 3);
        for (size_t i = 0; i < 3; i++) assert(count_entity(&world, ids[i]) == 1);
        entity_cell_loader_process(&world);
    }

    entity_cell_loader_free(world.ecell_loader);
    entity_cell_hash_map_free(&world.spatial.cells);
    ecs_destroy(&world.ecs);
    return 0;
}
