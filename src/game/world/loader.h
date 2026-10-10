#include "chunkmap.h"
#include "core/raster.h"
#include "world.h"

typedef VEC(ChunkPos) ChunkRequestQueue;

typedef struct {
    ChunkRequestQueue requests;
    size_t request_head;

    ChunkPos center;
    int rd_x;
    int rd_y;
    int rd_z;

    size_t gen_budget;
    size_t mesh_budget;
} ChunkLoader;

ChunkLoader *chunk_loader_new(World *world);

void chunk_loader_update(ChunkLoader *loader, World *world, Vec3d center);

bool chunk_loader_request(ChunkLoader *loader, World *world, ChunkPos pos);

void chunk_loader_process(ChunkLoader *loader, World *world);

void chunk_loader_unload_distant(ChunkLoader *loader, World *world);

void chunk_loader_free(ChunkLoader *loader);