#include "game.h"

#include <math.h>
#include <stdint.h>

#include "core/math.h"
#include "core/window.h"
#include "render/render.h"
#include "world/chunk_loader.h"
#include "world/ecell_loader.h"
#include "world/ecs.h"
#include "world/world.h"

#define PLAYER_SPEED 5.0

static void handle_input(Window *window);

void game_init(Window *window) {
    window->state = malloc_or_exit(sizeof(GameState));
    window->state->world = NULL;

    window->state->world = malloc_or_exit(sizeof(World));
    world_init(window->state->world);

    game_state_init(window->state);

    Vec3d placeholder_spawn_pos = {8, 8, 8};
    window->state->player_entity = world_join(window->state->world, placeholder_spawn_pos);
}

void game_destroy(Window *window) {
    if (window->state) {
        if (window->state->world) {
            world_destroy(window->state->world);
            free(window->state->world);
        }

        free(window->state);
    }
}

void game_update(Window *window) {
    GameState *state = window->state;
    float dt = window->dt;

    handle_input(window);
    world_update(state->world);
    Vec3d *player_pos = ECS_GET(&state->world->ecs, state->player_entity, position);
    chunk_loader_update(state->world, world_pos_to_chunk(*player_pos));
    chunk_loader_process(state->world);
    entity_cell_loader_process(state->world);
    update_camera(state);

    render_chunks(window);
}

static void handle_input(Window *window) {
    GameState *state = window->state;

    double dx = 0, dz = 0;
    if (window->input.keys[KC_W].is_down) dz -= 1;
    if (window->input.keys[KC_S].is_down) dz += 1;
    if (window->input.keys[KC_D].is_down) dx += 1;
    if (window->input.keys[KC_A].is_down) dx -= 1;

    double len = sqrt(dx * dx + dz * dz);
    if (len > 0) {
        dx /= len;
        dz /= len;
    }

    ECS_SET(&state->world->ecs, state->player_entity, input_vector, ((Vec2){dx, dz}));
}
