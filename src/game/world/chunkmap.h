#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "chunk.h"

#define CHUNK_MAP_INITIAL_CAPACITY 16
#define CHUNK_MAP_LOAD_NUMERATOR 3
#define CHUNK_MAP_LOAD_DENOMINATOR 4

typedef struct {
    int64_t x, y, z;
} ChunkPos;

typedef struct ChunkMapEntry ChunkMapEntry;
struct ChunkMapEntry {
    ChunkPos key;
    Chunk *value;
    ChunkMapEntry *next;  // for chaining
    bool valid;
};
;
typedef struct {
    ChunkMapEntry **entries;
    size_t count;
    size_t cap;
} ChunkHashMap;

typedef struct {
    size_t index;
    ChunkMapEntry *current;
} ChunkHashMapIterator;

ChunkHashMap chunk_hash_map_new(size_t cap);
void chunk_hash_map_insert(ChunkHashMap *map, ChunkPos key, Chunk *value);
Chunk *chunk_hash_map_get(const ChunkHashMap *map, ChunkPos key);
bool chunk_hash_map_remove(ChunkHashMap *map, ChunkPos key);
Chunk *chunk_hash_map_take(ChunkHashMap *map, ChunkPos key);
void chunk_hash_map_free(ChunkHashMap *map);
Chunk *chunk_hash_map_iter_next(ChunkHashMap *map, ChunkHashMapIterator *iter);

static inline int64_t chunk_floor_div(int64_t x) {
    int64_t q = x / 16;
    int64_t r = x % 16;
    return q - (r < 0);
}

static inline int64_t chunk_floor_mod(int64_t x) {
    int64_t r = x % 16;
    return r < 0 ? r + 16 : r;
}

static inline ChunkPos world_pos_to_chunk(Vec3d pos) {
    return (ChunkPos){
        .x = chunk_floor_div(pos.x),
        .y = chunk_floor_div(pos.y),
        .z = chunk_floor_div(pos.z),
    };
}