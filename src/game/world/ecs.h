#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "core/math.h"
#include "core/vec.h"

typedef uint64_t EntityId;

#define ENTITY_CELL_SIZE 32

#define ECS_COMPONENTS(X)           \
    X(POSITION, position, Vec3d, 0) \
    X(VELOCITY, velocity, Vec3, 1)  \
    X(INPUT_VEC, input_vector, Vec2, 2)

#define ECS_GEN_FLAG(name, field, type, bit) ECS_COM_##name = UINT64_C(1) << (bit),

typedef enum { ECS_COMPONENTS(ECS_GEN_FLAG) } EcsComponentFlags;

#undef ECS_GEN_FLAG

typedef VEC(EntityId) EntityList;

typedef struct {
    EntityId next_entity_id;
    EntityList entities;      // 実エンティティ ID の密なリスト
    VEC(size_t) id_to_index;  // entity ID -> entities/positions/velocities 内のインデックス
    VEC(EntityId) free_ids;   // 再利用可能な空 ID
    VEC(EcsComponentFlags) component_flags;  // エンティティごとのコンポーネントフラグ
} EntityManager;

// storage types
#define ECS_GEN_STORAGE(name, field, type, bit) typedef VEC(type) EcsStorage_##name;

ECS_COMPONENTS(ECS_GEN_STORAGE)

#undef ECS_GEN_STORAGE
// storage types end

typedef struct {
    EntityManager entities;

    // storage fields
#define ECS_GEN_FIELD(name, field, type, bit) EcsStorage_##name field;

    ECS_COMPONENTS(ECS_GEN_FIELD)

#undef ECS_GEN_FIELD
    // storage fields end

} ECS;

// Zero-initialize before use. required=0 matches all entities; otherwise all
// required components must be present. Do not create/destroy entities while
// iterating. Component values may be updated through ECS_GET/ECS_SET.
typedef struct {
    size_t next_index;
    EntityId current;
    EcsComponentFlags required;
} EcsIterator;

void ecs_init(ECS *ecs);
void ecs_destroy(ECS *ecs);
EntityId ecs_iter_next(const ECS *ecs, EcsIterator *iter);

EntityId ecs_create_entity(ECS *ecs);
void ecs_destroy_entity(ECS *ecs, EntityId id);
bool ecs_get_entity_index(const ECS *ecs, EntityId id, size_t *index);

int ecs_add_component(ECS *ecs, EntityId id, EcsComponentFlags flag);
int ecs_remove_component(ECS *ecs, EntityId id, EcsComponentFlags flag);
int ecs_has_component(const ECS *ecs, EntityId id, EcsComponentFlags flag);

#define ECS_GEN_ACCESSORS(name, field, type, bit)                                       \
    static inline type *__ecs_get_##field(ECS *ecs, EntityId id) {                      \
        size_t index;                                                                   \
        if (!ecs_get_entity_index(ecs, id, &index)) return NULL;                        \
        if (!(ecs->entities.component_flags.data[index] & ECS_COM_##name)) return NULL; \
        return &ecs->field.data[index];                                                 \
    }                                                                                   \
                                                                                        \
    static inline bool __ecs_set_##field(ECS *ecs, EntityId id, type value) {           \
        size_t index;                                                                   \
        if (!ecs_get_entity_index(ecs, id, &index)) return false;                       \
        if (ecs_add_component(ecs, id, ECS_COM_##name) != 0) return false;             \
        ecs->field.data[index] = value;                                                 \
        return true;                                                                    \
    }

ECS_COMPONENTS(ECS_GEN_ACCESSORS)

#undef ECS_GEN_ACCESSORS

#define ECS_GET(ecs, id, field) __ecs_get_##field((ecs), (id))

#define ECS_SET(ecs, id, field, value) __ecs_set_##field((ecs), (id), (value))
