#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "voxels.h"
#include <dirent.h>
#include <sys/stat.h>


int mod(int a, int b) {
  int r = a % b;
  return (r < 0) ? r + b : r;
}

voxel * getVoxelLocInWorld(struct world w, int x, int y, int z, struct chunk ** outChunk, struct region ** outRegion) {
  int regionX = floor(floor((double)x / CHUNK_SIZE) / (double)REGION_SIZE);
  int regionY = floor(floor((double)y / CHUNK_SIZE) / (double)REGION_SIZE);
  int regionZ = floor(floor((double)z / CHUNK_SIZE) / (double)REGION_SIZE);
  ivec3 chunkPosInRegion = (ivec3){
    mod((int)floor((double)x / CHUNK_SIZE) , REGION_SIZE),
    mod((int)floor((double)y / CHUNK_SIZE) , REGION_SIZE),
    mod((int)floor((double)z / CHUNK_SIZE) , REGION_SIZE)};
  ivec3 blockPosInChunk = {mod(x, CHUNK_SIZE), mod(y, CHUNK_SIZE), mod(z, CHUNK_SIZE) };

  struct region * checkRegion = w.loadedRegions;
  while (checkRegion) {
    if (regionX != checkRegion->x || regionY != checkRegion->y || regionZ != checkRegion->z) {
      checkRegion = checkRegion->next;
      continue;
    }
    struct chunk * checkChunk = checkRegion->chunks;
    while (checkChunk) {
      if (chunkPosInRegion.x != checkChunk->x || chunkPosInRegion.y != checkChunk->y || chunkPosInRegion.z != checkChunk->z) {
        checkChunk = checkChunk->next;
        continue;
      }
      unsigned index = (blockPosInChunk.z * CHUNK_SIZE * CHUNK_SIZE) + (blockPosInChunk.y * CHUNK_SIZE) + blockPosInChunk.x;
      if (outChunk) *outChunk = checkChunk;
      if (outRegion) *outRegion = checkRegion;
      return checkChunk->voxels+index;
    }
    return 0;
  }
  return 0;
}

voxel getVoxelInWorld(struct world w, int x, int y, int z) {
  
  voxel * loc = getVoxelLocInWorld(w,x,y,z,0,0);
  
  if (!loc) return 0;
  return * loc;
}

voxel setVoxelInWorld(struct world w, int x, int y, int z, voxel v) {
  struct chunk * chunk;
  struct region * r;
  voxel * loc = getVoxelLocInWorld(w,x,y,z,&chunk,&r);
  if (!loc) return 0;
  *loc = v;
  voxelMeshData(chunk, &w);
  r->hasBeenModified = true;
  return 1;
}

// used only by voxelMeshData()
voxel getVoxelInChunk(voxel * voxels, int x, int y, int z) {
  if (x < 0 || y < 0 || z < 0) { return 0; }
  if (x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) { return 0; }
  unsigned index = (z * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + x;
  return voxels[index];
}

unsigned voxelMeshData(struct chunk * chunk, struct world * world) {
  struct vertex { unsigned char x, y, z, w, vxl; };
  if (!chunk->vao) {
    glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
    glGenBuffers(1, &chunk->ebo);
  }
  // TODO: this is definitely not the right max size
  struct vertex * v = calloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 12, sizeof(struct vertex)); // can be static maybe?
  unsigned int * i  = calloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 18, sizeof(unsigned));
  unsigned char x, y, z;
  unsigned nV = 0;
  unsigned nI = 0;
 
  for (x = 0;x < CHUNK_SIZE; x++) {
    for (y = 0; y < CHUNK_SIZE; y++) {
      for (z = 0; z < CHUNK_SIZE; z++) {
        voxel vxl = getVoxelInChunk(chunk->voxels, x, y, z );
        if (!vxl) continue;
        
        if (!getVoxelInChunk(chunk->voxels, x+1, y  , z )) {
          v[nV] = (struct vertex){x+1, y  , z  , (0 * 6) + 0, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y+1, z  , (0 * 6) + 1, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y  , z+1, (0 * 6) + 2, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y+1, z+1, (0 * 6) + 3, vxl }; nV++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
        }
        
        if (!getVoxelInChunk(chunk->voxels, x-1, y  , z )) {
          
          v[nV] = (struct vertex){x, y  , z  , (1 * 6) + 0, vxl }; nV++;
          v[nV] = (struct vertex){x, y+1, z  , (1 * 6) + 1, vxl }; nV++;
          v[nV] = (struct vertex){x, y  , z+1, (1 * 6) + 2, vxl }; nV++;
          v[nV] = (struct vertex){x, y+1, z+1, (1 * 6) + 3, vxl }; nV++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
        }
        
        if (!getVoxelInChunk(chunk->voxels, x  , y+1, z   )) {
          v[nV] = (struct vertex){x  , y+1, z  , (2 * 6) + 0, vxl }; nV++;
          v[nV] = (struct vertex){x  , y+1, z+1, (2 * 6) + 1, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y+1, z  , (2 * 6) + 2, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y+1, z+1, (2 * 6) + 3, vxl }; nV++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
        }
        
        if (!getVoxelInChunk(chunk->voxels, x  , y-1, z   )) {
          v[nV] = (struct vertex){x  , y, z  , (3 * 6) + 0, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y, z  , (3 * 6) + 1, vxl }; nV++;
          v[nV] = (struct vertex){x  , y, z+1, (3 * 6) + 2, vxl }; nV++;
          v[nV] = (struct vertex){x+1, y, z+1, (3 * 6) + 3, vxl }; nV++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
        }
        
        if (!getVoxelInChunk(chunk->voxels, x  , y  , z+1)) {
          v[nV] = (struct vertex){x  , y  , z+1, (4 * 6) + 0, vxl}; nV++;
          v[nV] = (struct vertex){x  , y+1, z+1, (4 * 6) + 1, vxl}; nV++;
          v[nV] = (struct vertex){x+1, y  , z+1, (4 * 6) + 2, vxl}; nV++;
          v[nV] = (struct vertex){x+1, y+1, z+1, (4 * 6) + 3, vxl}; nV++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
        }
        
        if (!getVoxelInChunk(chunk->voxels, x  , y  , z-1 )) {
          v[nV] = (struct vertex){x  , y  , z  , (5 * 6) + 0, vxl}; nV++;
          v[nV] = (struct vertex){x  , y+1, z  , (5 * 6) + 1, vxl}; nV++;
          v[nV] = (struct vertex){x+1, y  , z  , (5 * 6) + 2, vxl}; nV++;
          v[nV] = (struct vertex){x+1, y+1, z  , (5 * 6) + 3, vxl}; nV++;
          i[nI] = nV-4; nI++;
          i[nI] = nV-3; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-1; nI++;
          i[nI] = nV-2; nI++;
          i[nI] = nV-3; nI++;
        }
      }
    }
  }
  
  glBindVertexArray(chunk->vao);    
  glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
  glBufferData(GL_ARRAY_BUFFER, nV * sizeof(struct vertex), v, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, nI * sizeof(unsigned int), i, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(struct vertex), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)3);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)4);
  glBindVertexArray(0);
  free(v);
  free(i);
  chunk->icount = nI;
  return nI;
}

int saveWorld(struct world * w) {
  // check if directory w->name exists
  // if not, create.
  mkdir(w->name, 0777);
  // for each region in the world
  struct region * r = w->loadedRegions;
  while (r) {
    if (r->hasBeenModified) {
      char filename[256];
      sprintf(filename, "%s/r%2d_%2d_%2d", w->name, r->x, r->y, r->z);
      FILE * fptr = fopen(filename,"wb");

      struct chunk * c = r->chunks;
      while (c) {
        struct chunk temp = *c;
        temp.vao = 0;
        temp.hasBeenModified = 0;
        fwrite(&temp, sizeof(struct chunk), 1, fptr);
        fwrite(c->voxels, sizeof(voxel), CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, fptr );
        c = c->next;
      }
      fclose(fptr);
    }
    r = r->next;
  }
}

int loadWorld(struct world *w) {
    struct stat st = {0};
    if (stat(w->name, &st) != 0) {
        printf("World directory '%s' does not exist\n", w->name);
        return -1;
    }
    DIR *dir = opendir(w->name);
    if (!dir) return -1;
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        if (ent->d_type != DT_REG) continue;
        if (ent->d_name[0] != 'r') continue;
        int rx, ry, rz;
        if (sscanf(ent->d_name, "r%d_%d_%d", &rx, &ry, &rz) != 3)
            continue;
        char fullpath[256];
        sprintf(fullpath, "%s/%s", w->name, ent->d_name);
        FILE *fptr = fopen(fullpath, "rb");
        if (!fptr) continue;
        struct region *region = calloc(1, sizeof(struct region));
        region->x = rx;
        region->y = ry;
        region->z = rz;
        region->hasBeenModified = 0;
        region->next = w->loadedRegions;
        w->loadedRegions = region;

        
        while (1) {
            struct chunk *chunk = malloc(sizeof(struct chunk));
            size_t n = fread(chunk, sizeof(struct chunk), 1, fptr);
            if (n != 1) {
                free(chunk);
                break; // EOF
            }
            // now read voxel data
            chunk->voxels = malloc(sizeof(voxel) *
                                   CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
            fread(chunk->voxels, sizeof(voxel),
                  CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE,
                  fptr);
            // link into region
            chunk->next = region->chunks;
            region->chunks = chunk;
        }
        fclose(fptr);
    }
    closedir(dir);
    return 0;
}


void generChunk(struct world * w, int x, int y, int z) {
  ivec3 regionPos = {floor((double)x/REGION_SIZE),floor((double)y/REGION_SIZE),floor((double)z/REGION_SIZE)};
  ivec3 chunkPosInRegion = {mod(x,REGION_SIZE),mod(y,REGION_SIZE),mod(z,REGION_SIZE)};  
  struct region * checkRegion = w->loadedRegions;
  int regionWasFound = 0;
  while (checkRegion) {
    if (checkRegion->x != regionPos.x || checkRegion->y != regionPos.y || checkRegion->z != regionPos.z) {
      checkRegion = checkRegion->next;
      continue;
    }
    checkRegion->hasBeenModified = true;  
    regionWasFound = 1;
    break;
  }
  if (!regionWasFound) {
    checkRegion = malloc(sizeof(struct region));
    checkRegion->next = w->loadedRegions;
    w->loadedRegions = checkRegion;
    checkRegion->x = regionPos.x; checkRegion->y = regionPos.y; checkRegion->z = regionPos.z;
  }
  struct chunk * newChunk = malloc(sizeof(struct chunk));
  *newChunk = (struct chunk){
    .x=chunkPosInRegion.x,
    .y=chunkPosInRegion.y,
    .z=chunkPosInRegion.z,
    .vao = 0,
    .vbo = 0,
    .ebo = 0,
    .next = checkRegion->chunks,
    .voxels = calloc(sizeof(voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, 1)
  };
  int perlin3d_binary(float x, float y, float z, float threshold);
  unsigned max = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
  float scale = 0.01f;       // controls "size" of terrain features
  float threshold = 0.0f;   // controls density
  for (int bx = 0; bx < CHUNK_SIZE; bx++) {
    for (int by = 0; by < CHUNK_SIZE; by++) {
      for (int bz = 0; bz < CHUNK_SIZE; bz++) {
        float fx = (x * CHUNK_SIZE + bx) * scale;
        float fy = (y * CHUNK_SIZE + by) * scale;
        float fz = (z * CHUNK_SIZE + bz) * scale;
        newChunk->voxels[(bz * CHUNK_SIZE * CHUNK_SIZE) + (by * CHUNK_SIZE) + bx] = perlin3d_binary(fx, fy, fz, threshold);
      }
    }
  }
  checkRegion->chunks = newChunk;
}

void * createWorld(void * arg) {
  struct world * w = (struct world *) arg;
  w->loadedRegions = NULL;
  unsigned chunkCount = WORLD_SIZE * WORLD_SIZE * WORLD_SIZE;
  for (int x = 0; x < (w->size_h); x++) {
    for (int y = 0; y < (w->size_v); y++) {
      for (int z = 0; z < (w->size_h); z++) {
        generChunk(w, x - (w->size_h / 2), y - (w->size_v / 2), z - (w->size_h / 2));
        w->creationProgress ++;
      }
    }
  }
  w->creationProgress = -1;
}

// yo shout out chat gpt for this one (i revised it a lil bit tho)
int voxelRaycastHit(
    struct world world,
    fvec3 o,
    float yaw, float pitch,
    float maxDist,
    ivec3 *hit
) {
    int success = 0;
    float dx = cosf(pitch) * cosf(yaw);
    float dy = sinf(pitch);
    float dz = cosf(pitch) * sinf(yaw);
    int x = floorf(o.x); int y = floorf(o.y); int z = floorf(o.z);
    float tMaxX, tMaxY, tMaxZ; float tDeltaX, tDeltaY, tDeltaZ;
    int stepX = (dx > 0) ? 1 : -1;
    int stepY = (dy > 0) ? 1 : -1;
    int stepZ = (dz > 0) ? 1 : -1;
    float rx = (dx == 0) ? 1e30f : fabsf(1.0f / dx);
    float ry = (dy == 0) ? 1e30f : fabsf(1.0f / dy);
    float rz = (dz == 0) ? 1e30f : fabsf(1.0f / dz);
    tMaxX = ((dx > 0 ? (x + 1) - o.x : o.x - x) * rx);
    tMaxY = ((dy > 0 ? (y + 1) - o.y : o.y - y) * ry);
    tMaxZ = ((dz > 0 ? (z + 1) - o.z : o.z - z) * rz);
    tDeltaX = rx; tDeltaY = ry; tDeltaZ = rz;
    float dist = 0.0f;
    while (dist <= maxDist) {
        if (getVoxelInWorld(world, x, y, z ) != 0) {
            *hit = (ivec3){x, y, z};
            return 1;
        }
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) { x += stepX; dist = tMaxX; tMaxX += tDeltaX; }
            else               { z += stepZ; dist = tMaxZ; tMaxZ += tDeltaZ; }
        } else {
            if (tMaxY < tMaxZ) { y += stepY; dist = tMaxY; tMaxY += tDeltaY; }
            else               { z += stepZ; dist = tMaxZ; tMaxZ += tDeltaZ; }
        }
    }
    return 0;
}
int voxelRaycastPlace(
    struct world world,
    fvec3 o,
    float yaw, float pitch,
    float maxDist,
    ivec3 * hit
){
    int success = false;
    float dx = cosf(pitch) * cosf(yaw);
    float dy = sinf(pitch);
    float dz = cosf(pitch) * sinf(yaw);
    int x = floorf(o.x); int y = floorf(o.y); int z = floorf(o.z);
    float tMaxX, tMaxY, tMaxZ; float tDeltaX, tDeltaY, tDeltaZ;
    int stepX = (dx > 0) ? 1 : -1;
    int stepY = (dy > 0) ? 1 : -1;
    int stepZ = (dz > 0) ? 1 : -1;
    float rx = (dx == 0) ? 1e30f : fabsf(1.0f / dx);
    float ry = (dy == 0) ? 1e30f : fabsf(1.0f / dy);
    float rz = (dz == 0) ? 1e30f : fabsf(1.0f / dz);
    tMaxX = ((dx > 0 ? (x + 1) - o.x : o.x - x) * rx);
    tMaxY = ((dy > 0 ? (y + 1) - o.y : o.y - y) * ry);
    tMaxZ = ((dz > 0 ? (z + 1) - o.z : o.z - z) * rz);
    tDeltaX = rx; tDeltaY = ry; tDeltaZ = rz;
    float dist = 0.0f;
    ivec3 lastEmpty = {x, y, z};
    while (dist <= maxDist) {
        if (getVoxelInWorld(world, x, y, z) != 0) {
            *hit = lastEmpty;
            return true;
        }
        lastEmpty = (ivec3){x, y, z};
        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) { x += stepX; dist = tMaxX; tMaxX += tDeltaX; }
            else               { z += stepZ; dist = tMaxZ; tMaxZ += tDeltaZ; }
        } else {
            if (tMaxY < tMaxZ) { y += stepY; dist = tMaxY; tMaxY += tDeltaY; }
            else               { z += stepZ; dist = tMaxZ; tMaxZ += tDeltaZ; }
        }
    }
    return 0;
}

// assumptions:
// world shader is active
// is supplied with projection and view matrices and texture

void drawWorld(struct world * w, fvec3 pos, int dh, int dv) {
  ivec3 chunkPos = (ivec3){floor((double)pos.x/CHUNK_SIZE), floor((double)pos.y/CHUNK_SIZE), floor((double)pos.z/CHUNK_SIZE)};
  for (int x = chunkPos.x - dh; x <= chunkPos.x + dh; x++) {
    for (int y = chunkPos.y - dv; y <= chunkPos.y + dv; y++) {
      for (int z = chunkPos.z - dh; z <= chunkPos.z + dh; z++) {
        int regionX = floor((double)x/REGION_SIZE);
        int regionY = floor((double)y/REGION_SIZE);
        int regionZ = floor((double)z/REGION_SIZE);
        ivec3 chunkPosInRegion = (ivec3){mod(x, REGION_SIZE), mod(y, REGION_SIZE), mod(z, REGION_SIZE)};
        struct region * checkRegion = w->loadedRegions;
        while (checkRegion) {
          if (regionX != checkRegion->x || regionY != checkRegion->y || regionZ != checkRegion->z) {
            checkRegion = checkRegion->next;
            continue;
          }
          struct chunk * checkChunk = checkRegion->chunks;
          while (checkChunk) {
            if (chunkPosInRegion.x != checkChunk->x || chunkPosInRegion.y != checkChunk->y || chunkPosInRegion.z != checkChunk->z) {
              checkChunk = checkChunk->next;
              continue;
            }
            unsigned modelUniformLoc = glGetUniformLocation(w->shaderProgram, "model");
            mat4 model; mat4Translate(model, x * CHUNK_SIZE, y * CHUNK_SIZE, z * CHUNK_SIZE); 
            glUniformMatrix4fv(modelUniformLoc, 1, GL_FALSE, model);
            if (!checkChunk->vao) {
              voxelMeshData(checkChunk, w);
            }
            glBindVertexArray(checkChunk->vao);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D_ARRAY, w->tex);
            glDrawElements(GL_TRIANGLES, checkChunk->icount, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            break;
          }
          break;
        }
      }
    }
  }
};