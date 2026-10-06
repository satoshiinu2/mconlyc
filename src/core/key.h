#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "min.h"

// clang-format off
/// X(名前,            Win32スキャンコード, evdev)
#define __KEY_DEF_LIST(X) \
X(ESCAPE,          0x001,   1)\
X(DIGIT1,          0x002,   2)\
X(DIGIT2,          0x003,   3)\
X(DIGIT3,          0x004,   4)\
X(DIGIT4,          0x005,   5)\
X(DIGIT5,          0x006,   6)\
X(DIGIT6,          0x007,   7)\
X(DIGIT7,          0x008,   8)\
X(DIGIT8,          0x009,   9)\
X(DIGIT9,          0x00A,  10)\
X(DIGIT0,          0x00B,  11)\
X(MINUS,           0x00C,  12)\
X(EQUAL,           0x00D,  13)\
X(BACKSPACE,       0x00E,  14)\
X(TAB,             0x00F,  15)\
\
X(Q,               0x010,  16)\
X(W,               0x011,  17)\
X(E,               0x012,  18)\
X(R,               0x013,  19)\
X(T,               0x014,  20)\
X(Y,               0x015,  21)\
X(U,               0x016,  22)\
X(I,               0x017,  23)\
X(O,               0x018,  24)\
X(P,               0x019,  25)\
X(BRACKET_LEFT,    0x01A,  26)\
X(BRACKET_RIGHT,   0x01B,  27)\
X(ENTER,           0x01C,  28)\
X(CONTROL_LEFT,    0x01D,  29)\
\
X(A,               0x01E,  30)\
X(S,               0x01F,  31)\
X(D,               0x020,  32)\
X(F,               0x021,  33)\
X(G,               0x022,  34)\
X(H,               0x023,  35)\
X(J,               0x024,  36)\
X(K,               0x025,  37)\
X(L,               0x026,  38)\
X(SEMICOLON,       0x027,  39)\
X(QUOTE,           0x028,  40)\
X(BACKQUOTE,       0x029,  41)\
X(SHIFT_LEFT,      0x02A,  42)\
X(BACKSLASH,       0x02B,  43)\
\
X(Z,               0x02C,  44)\
X(X,               0x02D,  45)\
X(C,               0x02E,  46)\
X(V,               0x02F,  47)\
X(B,               0x030,  48)\
X(N,               0x031,  49)\
X(M,               0x032,  50)\
X(COMMA,           0x033,  51)\
X(PERIOD,          0x034,  52)\
X(SLASH,           0x035,  53)\
X(SHIFT_RIGHT,     0x036,  54)\
X(NUMPAD_MULTIPLY, 0x037,  55)\
X(ALT_LEFT,        0x038,  56)\
X(SPACE,           0x039,  57)\
X(CAPS_LOCK,       0x03A,  58)\
\
X(F1,              0x03B,  59)\
X(F2,              0x03C,  60)\
X(F3,              0x03D,  61)\
X(F4,              0x03E,  62)\
X(F5,              0x03F,  63)\
X(F6,              0x040,  64)\
X(F7,              0x041,  65)\
X(F8,              0x042,  66)\
X(F9,              0x043,  67)\
X(F10,             0x044,  68)\
X(F11,             0x057,  87)\
X(F12,             0x058,  88)\
\
X(NUM_LOCK,        0x045,  69)\
X(SCROLL_LOCK,     0x046,  70)\
X(NUMPAD7,         0x047,  71)\
X(NUMPAD8,         0x048,  72)\
X(NUMPAD9,         0x049,  73)\
X(NUMPAD_SUBTRACT, 0x04A,  74)\
X(NUMPAD4,         0x04B,  75)\
X(NUMPAD5,         0x04C,  76)\
X(NUMPAD6,         0x04D,  77)\
X(NUMPAD_ADD,      0x04E,  78)\
X(NUMPAD1,         0x04F,  79)\
X(NUMPAD2,         0x050,  80)\
X(NUMPAD3,         0x051,  81)\
X(NUMPAD0,         0x052,  82)\
X(NUMPAD_DECIMAL,  0x053,  83)\
X(INTL_BACKSLASH,  0x056,  86)\
\
/* 日本語キーボード */ \
X(INTL_RO,         0x073,  89)\
X(CONVERT,         0x079,  92)\
X(KANA_MODE,       0x070,  93)\
X(NON_CONVERT,     0x07B,  94)\
X(INTL_YEN,        0x07D, 124)\
\
/* E0 拡張キー */ \
X(NUMPAD_ENTER,    0x11C,  96)\
X(CONTROL_RIGHT,   0x11D,  97)\
X(NUMPAD_DIVIDE,   0x135,  98)\
X(PRINT_SCREEN,    0x137,  99)\
X(ALT_RIGHT,       0x138, 100)\
X(HOME,            0x147, 102)\
X(ARROW_UP,        0x148, 103)\
X(PAGE_UP,         0x149, 104)\
X(ARROW_LEFT,      0x14B, 105)\
X(ARROW_RIGHT,     0x14D, 106)\
X(END,             0x14F, 107)\
X(ARROW_DOWN,      0x150, 108)\
X(PAGE_DOWN,       0x151, 109)\
X(INSERT,          0x152, 110)\
X(DELETE,          0x153, 111)\
X(META_LEFT,       0x15B, 125)\
X(META_RIGHT,      0x15C, 126)\
X(CONTEXT_MENU,    0x15D, 127)

// clang-format on

typedef enum {
    KC_NONE = 0,
#define X(n, w, e) KC_##n,
    __KEY_DEF_LIST(X)
#undef X
        KC_COUNT
} Key;

typedef enum { M_NONE = 0, M_LEFT, M_RIGHT, M_MIDDLE, M_COUNT } Mouse;

typedef struct {
    bool is_down;
    bool was_down;
    uint16_t press_count;
} KeyInfo;

typedef struct {
    KeyInfo keys[KC_COUNT];
    KeyInfo mouse_buttons[M_COUNT];
    UiVec2 mouse_pos;
    UiVec2 mouse_delta;
} Input;

typedef enum {
    ACTION_NONE = 0,
    ACTION_MOVE_FORWARD,
    ACTION_MOVE_BACKWARD,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_JUMP,
    ACTION_SPRINT,
    ACTION_CROUCH,
    ACTION_INTERACT,
    ACTION_PLACE_BLOCK,
    ACTION_BREAK_BLOCK,
    ACTION_INVENTORY,
    ACTION_COUNT
} Action;

typedef struct {
    Key action_to_key[ACTION_COUNT];
    Mouse action_to_mouse[ACTION_COUNT];
} KeyConfig;

void key_init(Input *input);  // 起動時に1回mapを生成
void handle_key(Input *input, Key k, bool is_down, bool is_repeat);
bool consume_key(Input *input, Key k);
void post_process_key(Input *input);
const char *key_name(Key k);

bool consume_action(Input *input, KeyConfig *config, Action a);

Key key_from_evdev(uint32_t code);  // Wayland: そのまま / X11: keycode-8
#ifdef _WIN32
#include <windows.h>
Key key_from_win32(LPARAM lparam);  // WM_KEYDOWN/UP, WM_SYSKEYDOWN/UP
#endif
