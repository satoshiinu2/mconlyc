#include "state.h"

#include "shader.h"

void game_state_init(GameState *state) {
    state->basic_shader = create_basic_shader();
}
