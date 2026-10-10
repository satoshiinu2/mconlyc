#include "state.h"

#include <stdio.h>
#include <stdlib.h>

#include "asset_ids.h"
#include "core/bmp.h"
#include "shader.h"

void game_state_init(GameState *state) {
    state->basic_shader = create_basic_shader();

    state->terrain_shader = create_terrain_shader();

    const Asset *stone_ph = asset_get(ASSET_STONE_BMP);
    if (parse_bitmap(stone_ph, &state->terrain_texture) != EXIT_SUCCESS) {
        printf("Failed to load texture\n");
    }
}
