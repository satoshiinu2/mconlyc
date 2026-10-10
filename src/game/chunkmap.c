#include "chunkmap.h"

#include "core/mem.h"

static uint32_t chunk_pos_hash(ChunkPos key) {
    uint32_t x = (uint32_t)key.x * UINT32_C(1597334677);
    uint32_t y = (uint32_t)key.y * UINT32_C(3817698767);
    uint32_t z = (uint32_t)key.z * UINT32_C(3369164213);

    uint32_t h = x ^ y ^ z;
    h ^= h >> 16;
    h *= UINT32_C(0x7feb352d);
    h ^= h >> 15;
    h *= UINT32_C(0x846ca68b);
    h ^= h >> 16;

    return h;
}

static bool chunk_pos_equal(ChunkPos a, ChunkPos b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static size_t chunk_hash_map_index(const ChunkHashMap *map, ChunkPos key) {
    return (size_t)chunk_pos_hash(key) % map->cap;
}

static size_t chunk_hash_map_load_limit(size_t size) {
    // floor(size * 3 / 4), without overflowing size * 3.
    return (size / 4) * 3 + ((size % 4) * 3) / 4;
}

// thanks gpt
static void chunk_hash_map_resize(ChunkHashMap *map, size_t new_count) {
    if (new_count == 0 || new_count > SIZE_MAX / sizeof(*map->entries)) {
        fprintf(stderr, "Fatal Error: Invalid size to resize hash map (%zu).\n", new_count);
        exit(EXIT_FAILURE);
    }

    ChunkMapEntry **new_entries = calloc_or_exit(new_count, sizeof(*new_entries));

    // Rehash every entry into the new bucket array.
    for (size_t i = 0; i < map->cap; i++) {
        ChunkMapEntry *entry = map->entries[i];

        while (entry) {
            ChunkMapEntry *next = entry->next;

            size_t index = (size_t)chunk_pos_hash(entry->key) % new_count;

            entry->next = new_entries[index];
            new_entries[index] = entry;

            entry = next;
        }
    }

    free(map->entries);

    map->entries = new_entries;
    map->cap = new_count;
}

ChunkHashMap *chunk_hash_map_new(size_t cap) {
    assert(cap > 0);

    ChunkHashMap *map = malloc_or_exit(sizeof *map);

    map->entries = calloc(cap, sizeof *map->entries);
    if (!map->entries) {
        free(map);
        abort();
    }

    map->cap = cap;
    map->count = 0;

    return map;
}

void chunk_hash_map_insert(ChunkHashMap *map, ChunkPos key, Chunk *value) {
    assert(map != NULL);

    size_t index = chunk_hash_map_index(map, key);

    // Update existing keys
    for (ChunkMapEntry *entry = map->entries[index]; entry != NULL; entry = entry->next) {
        if (chunk_pos_equal(entry->key, key)) {
            entry->value = value;
            return;
        }
    }

    // Grow before inserting a new key if needed.
    while (map->count >= chunk_hash_map_load_limit(map->cap)) {
        chunk_hash_map_resize(map, map->cap * 2);
    }

    index = chunk_hash_map_index(map, key);

    ChunkMapEntry *entry = malloc_or_exit(sizeof(*entry));

    *entry = (ChunkMapEntry){
        .key = key,
        .value = value,
        .next = map->entries[index],
    };

    map->entries[index] = entry;
    map->count++;
}

Chunk *chunk_hash_map_get(const ChunkHashMap *map, ChunkPos key) {
    assert(map != NULL);
    assert(map->cap > 0);

    size_t index = chunk_hash_map_index(map, key);

    for (ChunkMapEntry *entry = map->entries[index]; entry != NULL; entry = entry->next) {
        if (chunk_pos_equal(entry->key, key)) {
            return entry->value;
        }
    }

    return NULL;
}

bool chunk_hash_map_remove(ChunkHashMap *map, ChunkPos key) {
    assert(map != NULL);
    assert(map->cap > 0);

    size_t index = chunk_hash_map_index(map, key);

    ChunkMapEntry **entry = &map->entries[index];

    while (*entry) {
        if (chunk_pos_equal((*entry)->key, key)) {
            ChunkMapEntry *removed = *entry;

            *entry = removed->next;

            free(removed);
            map->count--;

            return true;
        }

        entry = &(*entry)->next;
    }

    return false;
}

void chunk_hash_map_free(ChunkHashMap *map) {
    if (!map) {
        return;
    }

    for (size_t i = 0; i < map->cap; i++) {
        ChunkMapEntry *entry = map->entries[i];

        while (entry) {
            ChunkMapEntry *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(map->entries);
    free(map);
}