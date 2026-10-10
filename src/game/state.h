#pragma once

#include "core/bmp.h"
#include "core/key.h"
#include "core/render.h"

typedef struct {
    KeyConfig key_config;

    Shader basic_shader;
    
    Shader terrain_shader;
    Texture terrain_texture;

} GameState;

void game_state_init(GameState *state);