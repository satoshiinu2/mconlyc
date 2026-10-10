#include "bmp.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "min.h"

typedef struct {
    uint16_t bfType;
    uint32_t bfOffBits;
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
} BMPHeader;

static inline pixel_t pack_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    return (pixel_t)((uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | ((uint32_t)a << 24));
}

static uint16_t read_u16_le(const uint8_t bytes[4]) {
    return (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
}

static uint32_t read_u32_le(const uint8_t bytes[4]) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

static int32_t read_i32_le(const uint8_t bytes[4]) {
    uint32_t u = read_u32_le(bytes);

    if (u <= INT32_MAX) {
        return (int32_t)u;
    }

    return -1 - (int32_t)(UINT32_MAX - u);
}

static long get_file_len(FILE *fp) {
    if (fseek(fp, 0, SEEK_END)) {
        perror("fseek failed");
        return -1;
    }

    long len = ftell(fp);

    if (fseek(fp, 0, SEEK_SET)) {
        perror("fseek failed");
        return -1;
    }

    if (len < 0) {
        perror("invalid file length");
        return -1;
    }

    if ((uintmax_t)len > SIZE_MAX) {
        perror("file too large");
        return -1;
    }

    return len;
}

static bool check_header(BMPHeader *h) {
    if (h->bfType != 0x4D42) {
        printf("invalid bfType %#x not supported\n", h->bfType);
        return false;
    }

    if (h->biSize != 40) {
        printf("biSize %d not supported\n", h->biSize);
        return false;
    }

    if (h->biPlanes != 1) {
        printf("invalid biPlanes %#x\n", h->biPlanes);
        return false;
    }

    if (h->biBitCount != 24) {
        printf("biBitCount %d not supported\n", h->biBitCount);
        return false;
    }

    if (h->biCompression != 0) {
        printf("biSize %" PRIu32 " not supported\n", h->biCompression);
        return false;
    }

    if (h->biWidth <= 0 || h->biHeight == 0) {
        printf("invalid size (%" PRId32 ", %" PRId32 ")\n", h->biWidth, h->biHeight);
        return false;
    }
    return true;
}

// pixelの所有権は渡す
result_t parse_bitmap(const Asset *asset, Texture *out) {
    *out = (Texture){.width = -1, .height = -1, .pixels = NULL};
    int result = EXIT_FAILURE;

    const uint8_t *fbuf = asset->data;
    pixel_t *pixels = NULL;

    BMPHeader header = {
        .bfType = read_u16_le(fbuf),
        .bfOffBits = read_u32_le(fbuf + 10),
        .biSize = read_u32_le(fbuf + 14),
        .biWidth = read_i32_le(fbuf + 18),
        .biHeight = read_i32_le(fbuf + 22),
        .biPlanes = read_u16_le(fbuf + 26),
        .biBitCount = read_u16_le(fbuf + 28),
        .biCompression = read_u32_le(fbuf + 30),
    };

#if IMG_PARSER_LOG
    printf("bfType: %d\n", header.bfType);
    printf("bfOffBits: %d\n", header.bfOffBits);
    printf("biSize: %" PRIu32 "\n", header.biSize);
    printf("biWidth: %d\n", header.biWidth);
    printf("biHeight: %d\n", header.biHeight);
    printf("biPlanes: %d\n", header.biPlanes);
    printf("biBitCount: %d\n", header.biBitCount);
    printf("biCompression: %d\n", header.biCompression);
#endif

    if (!check_header(&header)) {
        goto cleanup;
    }

    size_t row_bytes = (size_t)header.biWidth * 3;
    size_t stride = (row_bytes + 3) & ~(size_t)3;

    size_t width = (size_t)header.biWidth;
    // header.biHeightをint64_tに変換することでオーバーフロー回避
    size_t height =
        (size_t)(header.biHeight < 0 ? -(int64_t)header.biHeight : (int64_t)header.biHeight);

    // 範囲チェック
    if (height > SIZE_MAX / width) {
        fprintf(stderr, "image dimensions overflow\n");
        goto cleanup;
    }

    size_t count = width * height;

    if (count > SIZE_MAX / sizeof(*pixels)) {
        fprintf(stderr, "pixel buffer size overflow\n");
        goto cleanup;
    }

    pixels = malloc(count * sizeof(*pixels));
    if (!pixels) {
        perror("malloc error");
        goto cleanup;
    }

    for (size_t y = 0; y < height; ++y) {
        size_t file_y = header.biHeight > 0 ? height - 1 - y : y;
        const uint8_t *row = fbuf + header.bfOffBits + file_y * stride;

        for (size_t x = 0; x < width; ++x) {
            const uint8_t *src = row + x * 3;
            pixel_t *dst = &pixels[y * width + x];

            *dst = pack_rgba(src[2], src[1], src[0], 0xFF);
        }
    }

    *out = (Texture){.width = width, .height = height, .pixels = pixels};

    result = EXIT_SUCCESS;

cleanup:
    if (result != EXIT_SUCCESS && pixels != NULL) {
        free(pixels);
    }

    return result;
}