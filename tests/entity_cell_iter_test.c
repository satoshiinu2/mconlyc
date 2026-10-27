#include <assert.h>

#include "game/world/ecellmap.h"

int main(void) {
    ECS ecs;
    ecs_init(&ecs);
    EntityCell cell = {.entities = VEC_INIT(EntityId)};
    EcsIterator iter = {0};
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == EMPTY);

    EntityId outside = ecs_create_entity(&ecs);
    EntityId a = ecs_create_entity(&ecs);
    EntityId b = ecs_create_entity(&ecs);
    EntityId c = ecs_create_entity(&ecs);
    EntityId deleted = ecs_create_entity(&ecs);
    assert(ecs_add_component(&ecs, outside, ECS_COM_POSITION | ECS_COM_VELOCITY) == 0);
    assert(ecs_add_component(&ecs, a, ECS_COM_POSITION) == 0);
    assert(ecs_add_component(&ecs, b, ECS_COM_POSITION | ECS_COM_VELOCITY) == 0);
    VEC_PUSH(cell.entities, c);
    VEC_PUSH(cell.entities, deleted);
    VEC_PUSH(cell.entities, b);
    VEC_PUSH(cell.entities, UINT64_MAX);
    VEC_PUSH(cell.entities, a);
    ecs_destroy_entity(&ecs, deleted);

    iter = (EcsIterator){0};
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == c);
    assert(iter.current == c);
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == b);
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == a);
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == EMPTY);
    assert(iter.current == EMPTY);
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == EMPTY);

    iter = (EcsIterator){.required = ECS_COM_POSITION | ECS_COM_VELOCITY};
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == b);
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == EMPTY);

    iter = (EcsIterator){.required = ECS_COM_INPUT_VEC};
    assert(entity_cell_iter_next(&cell, &ecs, &iter) == EMPTY);
    VEC_FREE(cell.entities);
    ecs_destroy(&ecs);
    return 0;
}
