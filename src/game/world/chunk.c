#include "chunk.h"
#include <stdlib.h>


Chunk* chunk_new(){
    return malloc(sizeof(Chunk));
}
void chunk_destroy(Chunk*chunk){
    free(chunk);
}