#include "ecellmap.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "core/mem.h"

EntityId entity_cell_iter_next(const EntityCell *cell, const ECS *ecs, EcsIterator *iter) {
    while (iter->next_index < cell->entities.len) {
        EntityId id = cell->entities.data[iter->next_index++];
        size_t index;
        if (!ecs_get_entity_index(ecs, id, &index)) continue;
        if ((ecs->entities.component_flags.data[index] & iter->required) != iter->required)
            continue;
        return iter->current = id;
    }
    return iter->current = EMPTY;
}

static uint32_t entity_cell_pos_hash(EntityCellPos key) {
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

static bool entity_cell_pos_equal(EntityCellPos a, EntityCellPos b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static size_t entity_cell_hash_map_index(const EntityCellHashMap *map, EntityCellPos key) {
    return (size_t)entity_cell_pos_hash(key) % map->cap;
}

static size_t entity_cell_hash_map_load_limit(size_t size) {
    // floor(size * 3 / 4), without overflowing size * 3.
    return (size / 4) * 3 + ((size % 4) * 3) / 4;
}

// thanks gpt
static void entity_cell_hash_map_resize(EntityCellHashMap *map, size_t new_count) {
    if (new_count == 0 || new_count > SIZE_MAX / sizeof(*map->entries)) {
        fprintf(stderr, "Fatal Error: Invalid size to resize hash map (%zu).\n", new_count);
        exit(EXIT_FAILURE);
    }

    EntityCellMapEntry **new_entries = calloc_or_exit(new_count, sizeof(*new_entries));

    // Rehash every entry into the new bucket array.
    for (size_t i = 0; i < map->cap; i++) {
        EntityCellMapEntry *entry = map->entries[i];

        while (entry) {
            EntityCellMapEntry *next = entry->next;

            size_t index = (size_t)entity_cell_pos_hash(entry->key) % new_count;

            entry->next = new_entries[index];
            new_entries[index] = entry;

            entry = next;
        }
    }

    free(map->entries);

    map->entries = new_entries;
    map->cap = new_count;
}

EntityCellHashMap entity_cell_hash_map_new(size_t cap) {
    assert(cap > 0);

    EntityCellHashMap map = {0};

    map.entries = calloc_or_exit(cap, sizeof *map.entries);

    map.cap = cap;
    map.count = 0;

    return map;
}

void entity_cell_hash_map_insert(EntityCellHashMap *map, EntityCellPos key, EntityCell *value) {
    assert(map != NULL);

    size_t index = entity_cell_hash_map_index(map, key);

    // Update existing keys
    for (EntityCellMapEntry *entry = map->entries[index]; entry != NULL; entry = entry->next) {
        if (entity_cell_pos_equal(entry->key, key)) {
            entity_cell_hash_map_remove(map, key);
            break;
        }
    }

    // Grow before inserting a new key if needed.
    while (map->count >= entity_cell_hash_map_load_limit(map->cap)) {
        entity_cell_hash_map_resize(map, map->cap * 2);
    }

    index = entity_cell_hash_map_index(map, key);

    EntityCellMapEntry *entry = malloc_or_exit(sizeof(*entry));

    *entry = (EntityCellMapEntry){
        .key = key,
        .value = value,
        .next = map->entries[index],
    };

    map->entries[index] = entry;
    map->count++;
}

EntityCell *entity_cell_hash_map_get(const EntityCellHashMap *map, EntityCellPos key) {
    assert(map != NULL);
    assert(map->cap > 0);

    size_t index = entity_cell_hash_map_index(map, key);

    for (EntityCellMapEntry *entry = map->entries[index]; entry != NULL; entry = entry->next) {
        if (entity_cell_pos_equal(entry->key, key)) {
            return entry->value;
        }
    }

    return NULL;
}

EntityCell *entity_cell_hash_map_take(EntityCellHashMap *map, EntityCellPos key) {
    assert(map != NULL);
    assert(map->cap > 0);

    size_t index = entity_cell_hash_map_index(map, key);

    EntityCellMapEntry **entry = &map->entries[index];

    while (*entry) {
        if (entity_cell_pos_equal((*entry)->key, key)) {
            EntityCellMapEntry *removed = *entry;
            EntityCell *old = removed->value;

            *entry = removed->next;

            free(removed);
            map->count--;

            return old;
        }

        entry = &(*entry)->next;
    }

    return NULL;
}

bool entity_cell_hash_map_remove(EntityCellHashMap *map, EntityCellPos key) {
    EntityCell *old = entity_cell_hash_map_take(map, key);
    if (old) {
        VEC_FREE(old->entities);
        free(old);
        return true;
    }
    return false;
}

void entity_cell_hash_map_free(EntityCellHashMap *map) {
    if (!map) {
        return;
    }

    for (size_t i = 0; i < map->cap; i++) {
        EntityCellMapEntry *entry = map->entries[i];

        while (entry) {
            EntityCellMapEntry *next = entry->next;
            VEC_FREE(entry->value->entities);
            free(entry->value);
            free(entry);
            entry = next;
        }
    }

    free(map->entries);
    map->entries = NULL;
    map->cap = 0;
    map->count = 0;
}

EntityCell *entity_cell_hash_map_iter_next(EntityCellHashMap *map,
                                           EntityCellHashMapIterator *iter) {
    if (!map || !iter) {
        return NULL;
    }

    while (iter->index < map->cap) {
        EntityCellMapEntry *entry = map->entries[iter->index];

        if (entry) {
            if (iter->current == NULL) {
                iter->current = entry;
            } else {
                iter->current = iter->current->next;
            }

            if (iter->current) {
                return iter->current->value;
            }
        }

        iter->index++;
        iter->current = NULL;
    }

    return NULL;
}
