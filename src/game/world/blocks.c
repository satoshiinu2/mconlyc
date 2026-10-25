#include "blocks.h"



const BlockDef BLOCK_DEFS[] = {
    [BLOCK_AIR] = {.id = BLOCK_AIR, .name = "air", .opaque = false, .solid = false},
    [BLOCK_STONE] = {.id = BLOCK_STONE, .name = "stone", .opaque = true, .solid = true},
    [BLOCK_DIRT] = {.id = BLOCK_DIRT, .name = "dirt", .opaque = true, .solid = true},
    [BLOCK_GRASS] = {.id = BLOCK_GRASS, .name = "grass", .opaque = true, .solid = true},
};

const size_t BLOCK_DEFS_COUNT = sizeof(BLOCK_DEFS) / sizeof(BLOCK_DEFS[0]);