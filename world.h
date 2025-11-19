//
// Created by jack lewis on 11/18/25.
//

#ifndef WORLD_H
#define WORLD_H

#define REGION_SIZE 16
#define CHUNK_SIZE 16

typedef struct { int x, y;    } ivec2;
typedef struct { int x, y, z; } ivec3;

typedef unsigned char voxel_t;

typedef struct {
    ivec3 dimension;
    ivec3 positionOfRegion;
    ivec3 positionWithinRegion;
} compressedChunk_t;

typedef struct {
  int mutex;
  ivec3 positionWithinRegion;
  voxel_t * voxels;
} chunk_t;

void * chunkLoaderDaemon(void * arg);

typedef struct {
  ivec3 regionPosition;
  chunk_t * chunk;
} region_t;

typedef struct {
  int id;
  ivec3 sizeInChunks;
  ivec3 positionOfCenterChunk;
  region_t * regionLList;
} dimension_t;

typedef struct {
  
} world_t;

#endif //WORLD_H
