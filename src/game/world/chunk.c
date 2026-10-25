#include "chunk.h"

#include <stdlib.h>
#include <string.h>

Chunk *chunk_new() {
    Chunk *chunk = malloc(sizeof(Chunk));
    if (!chunk) return NULL;

    memset(chunk, 0, sizeof(*chunk));
    chunk->state = CHUNK_REQUESTED;

    chunk->blocks = malloc_or_exit(CHUNK_DATA_SIZE * sizeof(Blocks));
    for (size_t i = 0; i < CHUNK_DATA_SIZE; i++) {
        chunk->blocks[i] = BLOCK_AIR;
    }

    return chunk;
}

void chunk_destroy(Chunk *chunk) {
    if (!chunk) return;

    free(chunk->blocks);
    VEC_FREE(chunk->vertices);
    VEC_FREE(chunk->indices);

    free(chunk);
}
