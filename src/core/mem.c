#include "mem.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void *malloc_or_exit(size_t byte) {
    void *temp = malloc(byte);
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Failed to allocate buffer (%zu bytes).\n", byte);
        exit(EXIT_FAILURE);
    }

    return temp;
}

void *realloc_or_exit(void *ptr, size_t byte) {
    void *temp = realloc(ptr, byte);
    if (temp == NULL) {
        fprintf(stderr, "Fatal Error: Failed to reallocate buffer (%zu bytes).\n", byte);
        exit(EXIT_FAILURE);
    }

    return temp;
}
