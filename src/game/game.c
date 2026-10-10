#include "game.h"

#include <stdint.h>

#include "core/mem.h"
#include "core/render.h"
#include "core/window.h"
#include "shader.h"

void render_chunks(Window *window);

static const BasicVertexIn cubeVertices[] = {
    {{-1, -1, -1}},  //
    {{1, -1, -1}},   //
    {{1, 1, -1}},    //
    {{-1, 1, -1}},   //
    {{-1, -1, 1}},   //
    {{1, -1, 1}},    //
    {{1, 1, 1}},     //
    {{-1, 1, 1}},    //
};

static const int cubeIndices[] = {
    0, 1, 2, 0, 2, 3,  // 手前
    5, 4, 7, 5, 7, 6,  // 奥
    4, 0, 3, 4, 3, 7,  // 左
    1, 5, 6, 1, 6, 2,  // 右
    3, 2, 6, 3, 6, 7,  // 上
    4, 5, 1, 4, 1, 0,  // 下
};

void game_init(Window *window) {
    window->state = malloc_or_exit(sizeof(GameState));

    game_state_init(window->state);
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