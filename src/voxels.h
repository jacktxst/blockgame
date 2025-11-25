//
// Created by jack lewis on 11/24/25.
//

#ifndef VOXELS_H
#define VOXELS_H

#include "math.h"
#include <stdbool.h>

#define WORLD_SIZE 4
#define REGION_SIZE 16
#define CHUNK_SIZE 16
#define RENDER_DISTANCE 8
#define ANTIDENSITY 200

typedef unsigned char voxel;

#define BLOCK_ANIMATED 1
#define BLOCK_ROTATE_6 2
#define BLOCK_ROTATE_4 4

typedef struct {
    unsigned top, bottom, north, east, south, west;
    unsigned flags;
} blocktype;

struct chunk {
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

struct region {
    int x;
    int y;
    int z;

    bool hasBeenModified;
    
    struct chunk * chunks;
    struct region * next;
};

struct world {
    unsigned shaderProgram;
    unsigned creationProgress;
    struct region * loadedRegions;
    unsigned tex;
};

unsigned voxelMeshData(struct chunk * chunk);

voxel getVoxelInWorld(struct world w, int x, int y, int z);

voxel setVoxelInWorld(struct world w, int x, int y, int z, voxel v);

int voxelRaycastHit( struct world world, fvec3 o, float yaw, float pitch, float maxDist, ivec3 * hit );

int voxelRaycastPlace( struct world world, fvec3 o, float yaw, float pitch, float maxDist, ivec3 * hit );



#endif //VOXELS_H
