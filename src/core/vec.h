#include <assert.h>
#include <stddef.h>

// shut up
#include "mem.h"  // IWYU pragma: keep

#ifdef DEBUG
#define VEC_ASSERT_SIZE(vec)                                                                    \
    assert(sizeof((vec).data[0]) == (vec).p_debug_size &&                                       \
           "Vector type size mismatch. Ensure that the vector is initialized with VEC_INIT(T) " \
           "and that the type T matches the vector's data type.");
#else
#define VEC_ASSERT_SIZE(vec) ((void)0)
#endif

#ifdef DEBUG
#define VEC(T)               \
    struct {                 \
        T *data;             \
        size_t len;          \
        size_t cap;          \
        size_t p_debug_size; \
    }
#else
#define VEC(T)      \
    struct {        \
        T *data;    \
        size_t len; \
        size_t cap; \
    }
#endif

#ifdef DEBUG
#define VEC_INIT(T) {.p_debug_size = sizeof(T)}
#else
#define VEC_INIT(T) {0}
#endif

#define VEC_PUSH(vec, val)                                                            \
    do {                                                                              \
        VEC_ASSERT_SIZE(vec);                                                         \
        if ((vec).len >= (vec).cap) {                                                 \
            (vec).cap = (vec).cap ? (vec).cap * 2 : 8;                                \
            (vec).data = realloc_or_exit((vec).data, (vec).cap * sizeof *(vec).data); \
        }                                                                             \
        (vec).data[(vec).len++] = (val);                                              \
    } while (0)

#define VEC_GET(vec, index)                                           \
    do {                                                              \
        VEC_ASSERT_SIZE(vec);                                         \
        assert((size_t)(index) < (vec).len && "Index out of bounds"); \
    } while (0), (*(assert((size_t)(index) < (vec).len, &(vec).data[(index)])))

// Removes an element in O(1) by moving the last element into its slot.
// Order is not preserved. index is evaluated once; vec must have no side effects.
#define VEC_SWAP_REMOVE(vec, index)                                      \
    do {                                                                \
        VEC_ASSERT_SIZE(vec);                                           \
        size_t vec_swap_remove_index_ = (size_t)(index);                 \
        assert(vec_swap_remove_index_ < (vec).len && "Index out of bounds"); \
        size_t vec_swap_remove_last_ = (vec).len - 1;                    \
        if (vec_swap_remove_index_ != vec_swap_remove_last_) {           \
            (vec).data[vec_swap_remove_index_] = (vec).data[vec_swap_remove_last_]; \
        }                                                               \
        (vec).len = vec_swap_remove_last_;                              \
    } while (0)

#define VEC_FREE(vec)              \
    do {                           \
        free((vec).data);          \
        (vec).data = NULL;         \
        (vec).len = (vec).cap = 0; \
    } while (0)

#define VEC_CLEAR(vec) ((vec).len = 0)
