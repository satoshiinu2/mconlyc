#include "camera.h"

#include "game/world/ecs.h"
#include "state.h"

void update_camera(GameState *state) {
    Vec3d *position = ECS_GET(&state->world->ecs, state->player_entity, position);
    Vec3 *velocity = ECS_GET(&state->world->ecs, state->player_entity, velocity);

    state->camera.position = *position;
}