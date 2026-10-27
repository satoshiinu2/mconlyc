#pragma once

#include "camera.h"
#include "core/bmp.h"
#include "core/key.h"
#include "world/world.h"

typedef struct GameState{
    KeyConfig key_config;

    Texture terrain_texture;

    World *world;

    EntityId player_entity;

    CameraInfo camera;

} GameState;

void game_state_init(GameState *state);