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