#pragma once
#include <stddef.h>

#include "asset_ids.h"

typedef struct {
    const char *name;
    const unsigned char *data;
    size_t size;
} Asset;

extern const Asset assets[ASSET_COUNT];

static inline const Asset *asset_get(AssetId id) {
    return &assets[id];
}