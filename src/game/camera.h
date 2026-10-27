#pragma once

#include "core/math.h"

struct GameState;

typedef struct {
    Vec3d position;
} CameraInfo;

void update_camera(struct GameState *state);