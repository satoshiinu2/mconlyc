#include "window.h"

#include <stddef.h>

#include "game/game.h"
#include "key.h"

void window_upload_buffer(Window *window) {
    window->platform.upload_buffer(window->platform.native_handle, window->render_mgr.pixel_buffer,
                                   window->render_mgr.width, window->render_mgr.height);
}

void window_size_changed(Window *window) {
    resize_depth_buffer(&window->render_mgr);
}

void window_init(Window *window) {
    key_init(&window->input);
    game_init(window);
}
void window_update_frame(Window *window) {
    clear_render_buffers(&window->render_mgr);

    game_update(window);

    post_process_key(&window->input);

    window_upload_buffer(window);
}