#pragma once

#include <stdint.h>

typedef int result_t;
typedef uint8_t block_t;
typedef uint32_t pixel_t;
typedef void *uniform_t;
typedef const void *vertex_in_t;
typedef void *vertex_out_t;

#define CONCAT_INNER(a, b) a##b

#define WINDOW_TITLE "Minecraft on C without SDL"
#define WINDOW_WIDTH 854
#define WINDOW_HEIGHT 480

// 戻り値を捨てたら警告
#if defined(_MSC_VER)
#include <sal.h>
#define NODISCARD _Check_return_
#elif defined(__GNUC__) || defined(__clang__)
#define NODISCARD __attribute__((warn_unused_result))
#else
#define NODISCARD
#endif

typedef struct {
    int x;
    int y;
} UiVec2;

static inline pixel_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (pixel_t)((uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24));
}