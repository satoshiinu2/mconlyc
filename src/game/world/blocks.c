#include "blocks.h"

#define ICON_ALL_FACE(x) {x, x, x, x, x, x}

// fuck formatter
// clang-format off
const BlockDef BLOCK_DEFS[] = {
    [BLOCK_AIR] = {
        .id = BLOCK_AIR, 
        .name = "air",
        .icon = 0,
        .opaque = false,
        .solid = false
    },
    [BLOCK_STONE] = {
        .id = BLOCK_STONE,
        .name = "stone",
        .icon = ICON_ALL_FACE(1),
        .opaque = true,
        .solid = true
    },
    [BLOCK_DIRT] = {
        .id = BLOCK_DIRT, 
        .name = "dirt", 
        .icon = ICON_ALL_FACE(2), 
        .opaque = true, 
        .solid = true
    },
    [BLOCK_GRASS] = {
        .id = BLOCK_GRASS, 
        .name = "grass", 
        .icon = {2,0,3,3,3,3}, 
        .opaque = true, 
        .solid = true
    },
};
// clang-format on

const size_t BLOCK_DEFS_COUNT = sizeof(BLOCK_DEFS) / sizeof(BLOCK_DEFS[0]);