#pragma once

#include "core/key.h"
#include "core/render.h"

typedef struct {
    KeyConfig key_config;

    Shader basic_shader;

} GameState;

void game_state_init(GameState *state);