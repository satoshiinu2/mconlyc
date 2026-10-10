#pragma once

#include "asset.h"
#include "min.h"

typedef struct {
    uint32_t width;
    uint32_t height;
    pixel_t *pixels;
} Texture;

int parse_bitmap(const Asset *asset, Texture *out);