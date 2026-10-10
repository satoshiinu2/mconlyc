#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static inline void *malloc_or_exit(size_t bytes) {
    void *temp = malloc(bytes);
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Failed to allocate buffer (%zu bytes).\n", bytes);
        exit(EXIT_FAILURE);
    }

    return temp;
}

static inline void *realloc_or_exit(void *ptr, size_t bytes) {
    void *temp = realloc(ptr, bytes);
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Failed to reallocate buffer (%zu bytes).\n", bytes);
        exit(EXIT_FAILURE);
    }

    return temp;
}

static inline void *calloc_or_exit(size_t count, size_t align) {
    void *temp = calloc(count, align);
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Failed to zero allocate buffer (%zu bytes).\n", align);
        exit(EXIT_FAILURE);
    }

    return temp;
}