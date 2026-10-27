#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_STONE = 1,
    BLOCK_DIRT = 2,
    BLOCK_GRASS = 3,
} Blocks;

typedef struct {
    Blocks id;
    const char *name;
    const uint32_t icon[6];
    bool opaque;
    bool solid;
} BlockDef;

extern const BlockDef BLOCK_DEFS[];
extern const size_t BLOCK_DEFS_COUNT;
