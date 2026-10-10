#pragma once

#include "core/bmp.h"
#include "core/key.h"

typedef struct {
    KeyConfig key_config;

    Texture terrain_texture;

} GameState;

void game_state_init(GameState *state);