#include "ecs.h"

void ecs_init(ECS *ecs) {
    *ecs = (ECS){
        .entities = {
            .next_entity_id = 1,
            .entities = VEC_INIT(EntityId),
            .id_to_index = VEC_INIT(size_t),
            .free_ids = VEC_INIT(EntityId),
            .component_flags = VEC_INIT(EcsComponentFlags),
        },
#define ECS_INIT_STORAGE(name, field, type, bit) .field = VEC_INIT(type),
        ECS_COMPONENTS(ECS_INIT_STORAGE)
#undef ECS_INIT_STORAGE
    };
}

void ecs_destroy(ECS *ecs) {
#define ECS_FREE_STORAGE(name, field, type, bit) VEC_FREE(ecs->field);
    ECS_COMPONENTS(ECS_FREE_STORAGE)
#undef ECS_FREE_STORAGE
    VEC_FREE(ecs->entities.entities);
    VEC_FREE(ecs->entities.id_to_index);
    VEC_FREE(ecs->entities.free_ids);
    VEC_FREE(ecs->entities.component_flags);
    ecs_init(ecs);
}

bool ecs_get_entity_index(const ECS *ecs, EntityId id, size_t *index) {
    if (id >= ecs->entities.id_to_index.len) return false;
    size_t found = ecs->entities.id_to_index.data[id];
    if (found >= ecs->entities.entities.len) return false;
    if (ecs->entities.entities.data[found] != id) return false;
    *index = found;
    return true;
}

EntityId ecs_iter_next(const ECS *ecs, EcsIterator *iter) {
    while (iter->next_index < ecs->entities.entities.len) {
        size_t index = iter->next_index++;
        if ((ecs->entities.component_flags.data[index] & iter->required) != iter->required){
            continue;
        }
        return iter->current = ecs->entities.entities.data[index];
    }
    return iter->current = EMPTY;
}

static void ensure_id_to_index(EntityManager *mgr, EntityId id) {
    while (mgr->id_to_index.len <= id) {
        size_t invalid = SIZE_MAX;
        VEC_PUSH(mgr->id_to_index, invalid);
    }
}

EntityId ecs_create_entity(ECS *ecs) {
    EntityManager *mgr = &ecs->entities;
    EntityId id;

    // 空いている ID があれば再利用、なければ新規採番
    if (mgr->free_ids.len > 0) {
        id = mgr->free_ids.data[--mgr->free_ids.len];
    } else {
        id = mgr->next_entity_id++;
    }

    ensure_id_to_index(mgr, id);

    size_t index = mgr->entities.len;
    EcsComponentFlags no_flags = 0;

    VEC_PUSH(mgr->entities, id);

#define ECS_INIT_COMPONENT(name, field, type, bit) VEC_PUSH(ecs->field, ((type){0}));

    ECS_COMPONENTS(ECS_INIT_COMPONENT)

#undef ECS_INIT_COMPONENT

    VEC_PUSH(mgr->component_flags, no_flags);
    mgr->id_to_index.data[id] = index;

    return id;
}

void ecs_destroy_entity(ECS *ecs, EntityId id) {
    EntityManager *mgr = &ecs->entities;
    size_t index;
    if (!ecs_get_entity_index(ecs, id, &index)) return;
    size_t last = mgr->entities.len - 1;

    if (index != last) {
        EntityId last_id = mgr->entities.data[last];

        mgr->id_to_index.data[last_id] = index;
    }

#define ECS_SWAP_REMOVE(name, field, type, bit) VEC_SWAP_REMOVE(ecs->field, index);
    ECS_COMPONENTS(ECS_SWAP_REMOVE)
#undef ECS_SWAP_REMOVE

    VEC_SWAP_REMOVE(mgr->entities, index);
    VEC_SWAP_REMOVE(mgr->component_flags, index);

    // ID を空リストに戻す
    mgr->id_to_index.data[id] = SIZE_MAX;
    VEC_PUSH(mgr->free_ids, id);
}

int ecs_add_component(ECS *ecs, EntityId id, EcsComponentFlags flag) {
    size_t index;
    if (!ecs_get_entity_index(ecs, id, &index)) {
        return -1;
    }
    ecs->entities.component_flags.data[index] |= flag;
    return 0;
}

int ecs_remove_component(ECS *ecs, EntityId id, EcsComponentFlags flag) {
    size_t index;
    if (!ecs_get_entity_index(ecs, id, &index)) {
        return -1;
    }
    ecs->entities.component_flags.data[index] &= ~flag;
    return 0;
}

int ecs_has_component(const ECS *ecs, EntityId id, EcsComponentFlags flag) {
    size_t index;
    if (!ecs_get_entity_index(ecs, id, &index)) {
        return 0;
    }
    return (ecs->entities.component_flags.data[index] & flag) != 0;
}
