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
#define VEC_ASSERT_SIZE(vec) ()
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

#define VEC_FREE(vec)              \
    do {                           \
        free((vec).data);          \
        (vec).data = NULL;         \
        (vec).len = (vec).cap = 0; \
    } while (0)

#define VEC_CLEAR(vec) ((vec).len = 0)