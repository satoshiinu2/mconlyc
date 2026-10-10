#include <assert.h>
#include <stddef.h>

// shut up
#include "mem.h"  // IWYU pragma: keep

#define VEC(T)      \
    struct {        \
        T *data;    \
        size_t len; \
        size_t cap; \
    } 

#define VEC_INIT {0}

#define VEC_PUSH(vec, val)                                                            \
    do {                                                                              \
        if ((vec).len >= (vec).cap) {                                                 \
            (vec).cap = (vec).cap ? (vec).cap * 2 : 8;                                \
            (vec).data = realloc_or_exit((vec).data, (vec).cap * sizeof *(vec).data); \
        }                                                                             \
        (vec).data[(vec).len++] = (val);                                              \
    } while (0)

#define VEC_GET(vec, index) (*(assert((size_t)(index) < (vec).len), &(vec).data[(index)]))

#define VEC_FREE(vec)              \
    do {                           \
        free((vec).data);          \
        (vec).data = NULL;         \
        (vec).len = (vec).cap = 0; \
    } while (0)

#define VEC_CLEAR(vec) ((vec).len = 0)