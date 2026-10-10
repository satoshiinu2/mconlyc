#include "game.h"

#include <stdint.h>

#include "core/mem.h"
#include "core/render.h"
#include "core/window.h"
#include "render/render.h"
#include "world/world.h"

void game_init(Window *window) {
    window->state = malloc_or_exit(sizeof(GameState));

    game_state_init(window->state);

    window->state->world = malloc_or_exit(sizeof(World));

    world_init(window->state->world);
}

void game_destroy(Window *window) {
    if (window->state) {
        free(window->state);
    }
}

void game_update(Window *window) {
    uint32_t width = window->render_mgr.width;
    uint32_t height = window->render_mgr.height;

    render_chunks(window);
}