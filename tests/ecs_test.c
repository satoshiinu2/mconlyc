#include <assert.h>

#include "game/world/ecs.h"

int main(void) {
    ECS storage;
    ECS *ecs = &storage;
    ecs_init(ecs);
    EcsIterator empty = {0};
    assert(ecs_iter_next(ecs, &empty) == EMPTY);

    EntityId a = ecs_create_entity(ecs);
    EntityId b = ecs_create_entity(ecs);
    EntityId c = ecs_create_entity(ecs);
    assert(ECS_SET(ecs, a, position, ((Vec3d){1, 2, 3})));
    assert(ECS_SET(ecs, b, position, ((Vec3d){4, 5, 6})));
    assert(ECS_SET(ecs, b, velocity, ((Vec3){7, 8, 9})));
    assert(ECS_GET(ecs, a, position)->x == 1);

    EcsIterator all = {0};
    EntityId expected[] = {a, b, c};
    for (size_t i = 0; i < 3; ++i) {
        assert(ecs_iter_next(ecs, &all) == expected[i]);
        assert(all.current == expected[i]);
    }
    assert(ecs_iter_next(ecs, &all) == EMPTY);
    assert(ecs_iter_next(ecs, &all) == EMPTY);

    EcsIterator filtered = {.required = ECS_COM_POSITION | ECS_COM_VELOCITY};
    assert(ecs_iter_next(ecs, &filtered) == b);
    assert(filtered.current == b);
    assert(ecs_iter_next(ecs, &filtered) == EMPTY);

    assert(ECS_SET(ecs, c, position, ((Vec3d){10, 11, 12})));
    assert(ECS_SET(ecs, c, input_vector, ((Vec2){2, 3})));

    ecs_destroy_entity(ecs, a);
    assert(ecs->position.len == 2 && ecs->velocity.len == 2 && ecs->input_vector.len == 2);
    assert(ECS_GET(ecs, b, position)->x == 4);
    assert(ECS_GET(ecs, c, position)->x == 10);
    assert(ECS_GET(ecs, c, input_vector)->x == 2);
    assert(ECS_GET(ecs, c, velocity) == NULL);
    assert(ECS_GET(ecs, a, position) == NULL);
    ecs_destroy_entity(ecs, a);
    ecs_destroy_entity(ecs, UINT64_MAX);
    assert(ecs_add_component(ecs, UINT64_MAX, ECS_COM_POSITION) == -1);

    EntityId reused = ecs_create_entity(ecs);
    assert(reused == a);
    assert(ECS_GET(ecs, reused, position) == NULL);
    ecs_destroy_entity(ecs, reused);
    ecs_destroy_entity(ecs, b);
    ecs_destroy_entity(ecs, c);
    assert(ecs->position.len == 0 && ecs->velocity.len == 0 && ecs->input_vector.len == 0);
    ecs_destroy(ecs);
    return 0;
}
