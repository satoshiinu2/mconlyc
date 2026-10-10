#include "render.h"

#include "core/window.h"
#include "game/state.h"
#include "shader.h"

static const TerrainVertexIn cubeVertices[] = {
    {{-1, -1, -1}, {0, 1}},  //
    {{1, -1, -1}, {1, 1}},   //
    {{1, 1, -1}, {1, 0}},    //
    {{-1, 1, -1}, {0, 0}},   //
    {{-1, -1, 1}, {0, 1}},   //
    {{1, -1, 1}, {1, 1}},    //
    {{1, 1, 1}, {1, 0}},     //
    {{-1, 1, 1}, {0, 0}},    //
};

static const int cubeIndices[] = {
    0, 1, 2, 0, 2, 3,  // 手前
    5, 4, 7, 5, 7, 6,  // 奥
    4, 0, 3, 4, 3, 7,  // 左
    1, 5, 6, 1, 6, 2,  // 右
    3, 2, 6, 3, 6, 7,  // 上
    4, 5, 1, 4, 1, 0,  // 下
};

void render_chunks(Window *window) {
    uint32_t width = window->render_mgr.width;
    uint32_t height = window->render_mgr.height;

    Mat4 model = mat4_identity();
    Mat4 view = mat4_translate(0, 0, -5);
    Mat4 proj = mat4_perspective(1.0f, (float)width / height, NEAR_VAL, 100.0f);
    Mat4 vp = mat4_viewport(width, height);
    Mat4 mvp = mat4_mul(vp, mat4_mul(proj, mat4_mul(view, model)));

    GameState *state = window->state;

    TerrainUniforms uniforms = {.texture = &state->terrain_texture, .mvp = &mvp};

    const size_t cubeIndexCount = sizeof(cubeIndices) / sizeof(cubeIndices[0]);

    render_elements_terrain(&window->render_mgr, &uniforms, NEAR_VAL, cubeVertices, cubeIndices,
                            cubeIndexCount);
}