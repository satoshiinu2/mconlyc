#pragma once

#include <stdint.h>

#include "game/state.h"
#include "key.h"
#include "min.h"
#include "render.h"

typedef struct {
    // HWND, xdg_toplevel
    void *native_handle;

    void (*upload_buffer)(void *native_handle, pixel_t *pixel_buffer, uint32_t width,
                          uint32_t height);
} WindowPlatform;

typedef struct {
    RenderManager render_mgr;
    WindowPlatform platform;
    Input input;

    // 起動するまでNULL
    GameState *state;

    // プラットフォーム依存側で更新しておく必要がある
    double time;
    float dt;
} Window;

void window_upload_buffer(Window *window);

// render_mgrのpixel_buffer, width, heightはこの関数が呼ばれる前に更新しておく必要がある
void window_size_changed(Window *window);
void window_init(Window *window);
void window_update_frame(Window *window);