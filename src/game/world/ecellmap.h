#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/math.h"
#include "core/vec.h"
#include "ecs.h"

typedef struct {
    int64_t x, y, z;
} EntityCellPos;

typedef struct EntityCell EntityCell;

typedef struct EntityCellMapEntry EntityCellMapEntry;
struct EntityCellMapEntry {
    EntityCellPos key;
    EntityCell *value;
    EntityCellMapEntry *next;  // for chaining
    bool valid;
};

typedef struct {
    EntityCellMapEntry **entries;
    size_t count;
    size_t cap;
} EntityCellHashMap;

typedef struct {
    EntityCellHashMap cells;
} EntitySpatialIndex;

typedef struct {
    size_t index;
    EntityCellMapEntry *current;
} EntityCellHashMapIterator;

static inline int64_t entity_cell_floor_div(int64_t x) {
    int64_t q = x / ENTITY_CELL_SIZE;
    int64_t r = x % ENTITY_CELL_SIZE;
    return q - (r < 0);
}

static inline int64_t entity_cell_floor_mod(int64_t x) {
    int64_t r = x % ENTITY_CELL_SIZE;
    return r < 0 ? r + ENTITY_CELL_SIZE : r;
}

static inline EntityCellPos world_pos_to_entity_cell(Vec3d pos) {
    return (EntityCellPos){
        .x = entity_cell_floor_div(pos.x),
        .y = entity_cell_floor_div(pos.y),
        .z = entity_cell_floor_div(pos.z),
    };
}

struct EntityCell {
    EntityList entities;
};

// Zero-initialize iter; required=0 matches all live entities in this cell.
// All required component flags must match. Returns EMPTY when exhausted.
// Do not modify the cell's entity list or create/destroy entities while iterating.
// The cell and ECS must stay the same until the iterator is reset.
EntityId entity_cell_iter_next(const EntityCell *cell, const ECS *ecs, EcsIterator *iter);

EntityCellHashMap entity_cell_hash_map_new(size_t cap);
void entity_cell_hash_map_insert(EntityCellHashMap *map, EntityCellPos key, EntityCell *value);
EntityCell *entity_cell_hash_map_get(const EntityCellHashMap *map, EntityCellPos key);
bool entity_cell_hash_map_remove(EntityCellHashMap *map, EntityCellPos key);
EntityCell *entity_cell_hash_map_take(EntityCellHashMap *map, EntityCellPos key);
void entity_cell_hash_map_free(EntityCellHashMap *map);
EntityCell *entity_cell_hash_map_iter_next(EntityCellHashMap *map, EntityCellHashMapIterator *iter);
