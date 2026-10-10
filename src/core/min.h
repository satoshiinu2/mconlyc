#pragma once

#include <stdint.h>

typedef int result_t;
typedef uint32_t pixel_t;
typedef void *uniform_t;
typedef const void *vertex_in_t;
typedef void *vertex_out_t;

#define WINDOW_TITLE "Minecraft on C without SDL"
#define WINDOW_WIDTH 854
#define WINDOW_HEIGHT 480

typedef struct {
    int x;
    int y;
} UiVec2;
