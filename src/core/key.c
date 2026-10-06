#include "key.h"

#include <stdint.h>

static const struct {
    uint16_t win, evdev;
} k_src[] = {{0, 0},  // KC_NONE
#define X(n, w, e) {w, e},
             __KEY_DEF_LIST(X)
#undef X
};

// 直接読まずにkey_nameを使って
static const char *k_names[] = {"None",
#define X(n, w, e) #n,
                                __KEY_DEF_LIST(X)
#undef X
};

static uint16_t k_from_evdev[256];
static uint16_t k_from_win[0x200];

void key_init(Input *input) {
    for (int k = 1; k < KC_COUNT; k++) {
        if (k_src[k].evdev) k_from_evdev[k_src[k].evdev] = (uint16_t)k;
        if (k_src[k].win) k_from_win[k_src[k].win] = (uint16_t)k;
    }

    // Initialize input state
    for (int k = 0; k < KC_COUNT; k++) {
        input->keys[k].is_down = false;
        input->keys[k].was_down = false;
        input->keys[k].press_count = 0;
    }
    for (int mb = 0; mb < M_COUNT; mb++) {
        input->mouse_buttons[mb].is_down = false;
        input->mouse_buttons[mb].was_down = false;
        input->mouse_buttons[mb].press_count = 0;
    }
    input->mouse_pos.x = 0;
    input->mouse_pos.y = 0;
    input->mouse_delta.x = 0;
    input->mouse_delta.y = 0;
}

void handle_key(Input *input, Key k, bool is_down, bool is_repeat) {
    if (k >= KC_COUNT) {
        return;
    }

    KeyInfo *info = &input->keys[k];

    if (is_down) {
        info->is_down = true;
        info->press_count++;
    } else {
        info->is_down = false;
    }
}

bool consume_key(Input *input, Key k) {
    if (k >= KC_COUNT) {
        return false;
    }

    KeyInfo *info = &input->keys[k];

    if (info->press_count == 0) {
        return false;
    }

    info->press_count--;
    return true;
}

void post_process_key(Input *input) {
    for (int k = 1; k < KC_COUNT; k++) {
        input->keys[k].was_down = input->keys[k].is_down;
        input->keys[k].press_count = 0;
    }
    for (int mb = 0; mb < M_COUNT; mb++) {
        input->mouse_buttons[mb].was_down = input->mouse_buttons[mb].is_down;
        input->mouse_buttons[mb].press_count = 0;
    }
    input->mouse_delta.x = 0;
    input->mouse_delta.y = 0;
}

const char *key_name(Key k) {
    return k < KC_COUNT ? k_names[k] : "?";
}

bool consume_action(Input *input, KeyConfig *config, Action a) {
    if (a >= ACTION_COUNT) {
        return false;
    }

    Key k = config->action_to_key[a];
    if (k != KC_NONE) {
        return consume_key(input, k);
    }

    Mouse mb = config->action_to_mouse[a];
    if (mb != M_NONE) {
        if (mb >= M_COUNT) {
            return false;
        }
        KeyInfo *info = &input->mouse_buttons[mb];
        if (info->press_count == 0) {
            return false;
        }
        info->press_count--;
        return true;
    }

    return false;
}

Key key_from_evdev(uint32_t c) {
    return c < 256 ? (Key)k_from_evdev[c] : KC_NONE;
}

#ifdef _WIN32
Key key_from_win32(LPARAM lp) {
    unsigned sc = (lp >> 16) & 0xFF;
    unsigned ext = (lp >> 24) & 1;
    return (Key)k_from_win[(ext << 8) | sc];
}
#endif