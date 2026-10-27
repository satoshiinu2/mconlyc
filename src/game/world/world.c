#include "world.h"

#include "chunk_loader.h"
#include "core/math.h"
#include "ecell_loader.h"
#include "ecellmap.h"
#include "ecs.h"
#include "entity.h"
#include "game/world/chunkmap.h"

void world_init(World *world) {
    world->chunks = chunk_hash_map_new(5);

    ecs_init(&world->ecs);

    world->spatial = (EntitySpatialIndex){
        .cells = entity_cell_hash_map_new(5),
    };

    world->chunk_loader = chunk_loader_new(world);
    world->ecell_loader = entity_cell_loader_new(world);
}

// returns player entity id
EntityId world_join(World *world, Vec3d player_pos) {
    ChunkPos player_chunk = world_pos_to_chunk(player_pos);
    EntityCellPos player_cell = world_pos_to_entity_cell(player_pos);

    EntityId id = ecs_create_entity(&world->ecs);
    ECS_SET(&world->ecs, id, position, player_pos);
    ECS_SET(&world->ecs, id, velocity, (Vec3){0});
    ECS_SET(&world->ecs, id, input_vector, (Vec2){0});

    entity_cell_loader_add_entity(world, player_cell, id, true);

    entity_cell_loader_update(world, player_cell);
    chunk_loader_update(world, player_chunk);

    return id;
}

void world_update(World *world) {
    update_entities(world);
}

void world_destroy(World *world) {
    if (!world) {
        return;
    }

    if (world->chunk_loader) {
        chunk_loader_free(world->chunk_loader);
        world->chunk_loader = NULL;
    }

    if (world->ecell_loader) {
        entity_cell_loader_free(world->ecell_loader);
        world->ecell_loader = NULL;
    }

    chunk_hash_map_free(&world->chunks);

    // ECS ストレージ解放
    ecs_destroy(&world->ecs);

    // 空間インデックス解放
    entity_cell_hash_map_free(&world->spatial.cells);
}
