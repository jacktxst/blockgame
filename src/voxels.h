//
// Created by jack lewis on 11/24/25.
//

#ifndef VOXELS_H
#define VOXELS_H

#include "math.h"
#include <stdbool.h>

#define WORLD_SIZE 8
#define REGION_SIZE 8
#define CHUNK_SIZE 64
#define ANTIDENSITY 200


#define CHUNK_QUEUE_SIZE 2048

struct chunk_queue {
    int front, rear;
    struct chunk * chunkRefArray[CHUNK_QUEUE_SIZE];
};

int enqueueChunk(struct chunk_queue *queue, struct chunk *chunk);
struct chunk *dequeueChunk(struct chunk_queue *queue);


typedef unsigned char voxel;

struct chunk_file {
    unsigned x, y, z;
};

struct legacy_chunk {
    unsigned x;
    unsigned y;
    unsigned z;
    
    unsigned vao;
    unsigned vbo;
    unsigned ebo;
    unsigned icount;
    
    bool needsRemesh;
    bool hasBeenModified;
    bool isCompressed;
    bool isLocked;
    voxel * voxels;
    struct chunk * next;
};

struct chunk {
    unsigned x;
    unsigned y;
    unsigned z;
    
    unsigned vao, vao_transparent;
    unsigned vbo, vbo_transparent;
    unsigned ebo, ebo_transparent;
    unsigned icount, icount_transparent;

    void * parentWorld;
    void * parentRegion;
    
    bool needsRemesh;
    bool inMeshQueue;
    bool hasBeenModified;
    bool isCompressed;
    bool isLocked;
    voxel * voxels;
    struct chunk * next;
};

struct region {
    int x;
    int y;
    int z;

    bool hasBeenModified;
    
    struct chunk * chunks;
    struct region * next;
};

struct world {
    unsigned * block_tex_lut;
    unsigned size_block_tex_lut;
    unsigned shaderProgram;
    unsigned creationProgress;
    unsigned size_v;
    unsigned size_h;
    struct region * loadedRegions;
    unsigned tex;
    char * name;
};

unsigned voxelMeshData(struct chunk * chunk, struct world * world);

voxel getVoxelInWorld(struct world w, int x, int y, int z);

voxel setVoxelInWorld(struct world * w, int x, int y, int z, voxel v, struct chunk_queue * meshing_queue);

int voxelRaycastHit( struct world world, fvec3 o, float yaw, float pitch, float maxDist, ivec3 * hit );

int voxelRaycastPlace( struct world world, fvec3 o, float yaw, float pitch, float maxDist, ivec3 * hit );

int saveWorld(struct world * w);

int loadWorld(struct world * w);

void freeWorld(struct world *w);

#endif //VOXELS_H
