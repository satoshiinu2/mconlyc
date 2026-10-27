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

    GameState *state = window->state;
    CameraInfo *camera = &state->camera;

    Mat4 model = mat4_identity();
    Mat4 view = mat4_translate(-camera->position.x, -camera->position.y, -camera->position.z);
    Mat4 proj = mat4_perspective(1.0f, (float)width / height, NEAR_VAL, 100.0f);
    Mat4 vp = mat4_viewport(width, height);
    Mat4 mvp = mat4_mul(vp, mat4_mul(proj, mat4_mul(view, model)));

    ChunkHashMapIterator iter = {0};
    while (chunk_hash_map_iter_next(&state->world->chunks, &iter)) {
        Chunk *chunk = iter.current->value;
        if (chunk->state != CHUNK_READY) continue;
        ChunkPos pos = iter.current->key;
        model = mat4_translate((double)pos.x * CHUNK_SIZE, (double)pos.y * CHUNK_SIZE,
                               (double)pos.z * CHUNK_SIZE);
        mvp = mat4_mul(vp, mat4_mul(proj, mat4_mul(view, model)));
        TerrainUniforms uniforms = {.texture = &state->terrain_texture, .mvp = &mvp};

        TerrainVertexIn *vertices = chunk->vertices.data;
        int *indices = chunk->indices.data;
        size_t index_count = chunk->indices.len;

        render_elements_terrain(&window->render_mgr, &uniforms, NEAR_VAL, vertices, indices,
                                index_count);
    }
}
