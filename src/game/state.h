#pragma once

#include "core/bmp.h"
#include "core/key.h"
#include "world/world.h"

typedef struct {
    KeyConfig key_config;

    Texture terrain_texture;

    World *world;

} GameState;

void game_state_init(GameState *state);