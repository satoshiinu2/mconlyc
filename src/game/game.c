#include "game.h"

#include <stdint.h>
#include <stdio.h>

#include "core/mem.h"
#include "core/min.h"
#include "core/render.h"
#include "core/window.h"
#include "shader.h"

/** @type {BasicVertexIn[]} */
const BasicVertexIn cubeVertices[] = {
    {{-1, -1, -1}},  //
    {{1, -1, -1}},   //
    {{1, 1, -1}},    //
    {{-1, 1, -1}},   //
    {{-1, -1, 1}},   //
    {{1, -1, 1}},    //
    {{1, 1, 1}},     //
    {{-1, 1, 1}},    //
};

const int cubeIndices[] = {
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

    BasicUniforms uniforms = {.scale = min(width, height) / 2,
                              .z_offset = 5,
                              .width = width,
                              .height = height,
                              .time = window->time};

    const size_t cubeIndexCount = sizeof(cubeIndices) / sizeof(cubeIndices[0]);

    for (size_t i = 0; i < cubeIndexCount; i += 3) {
        int i1 = cubeIndices[i];
        int i2 = cubeIndices[i + 1];
        int i3 = cubeIndices[i + 2];

        render_triangle(&window->render_mgr, &window->state->basic_shader, (uniform_t)&uniforms,
                        0.1, (vertex_in_t)&cubeVertices[i1], (vertex_in_t)&cubeVertices[i2],
                        (vertex_in_t)&cubeVertices[i3]);
    }
}