#include <assert.h>
#include <stddef.h>

// shut up
#include "mem.h"  // IWYU pragma: keep

#define VEC(T)      \
    struct {        \
        T *ptr;     \
        size_t len; \
        size_t cap; \
    }  // capが-1ならfrozen

#define VEC_INIT {0}

#define VEC_PUSH(vec, val)                                                         \
    do {                                                                           \
        if ((vec).len >= (vec).cap) {                                              \
            (vec).cap = (vec).cap ? (vec).cap * 2 : 8;                             \
            (vec).ptr = realloc_or_exit((vec).ptr, (vec).cap * sizeof *(vec).ptr); \
        }                                                                          \
        (vec).ptr[(vec).len++] = (val);                                            \
    } while (0)

#define VEC_GET(vec, index) (*(assert((size_t)(index) < (vec).len), &(vec).ptr[(index)]))

#define VEC_FREE(vec)              \
    do {                           \
        free((vec).ptr);           \
        (vec).ptr = NULL;          \
        (vec).len = (vec).cap = 0; \
    } while (0)

#define VEC_FREEZE(vec) ((vec).cap = -1, (vec).ptr)