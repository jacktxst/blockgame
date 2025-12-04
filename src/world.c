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



int enqueueChunk(struct chunk_queue *queue, struct chunk *chunk) {
  int next = (queue->rear + 1) % CHUNK_QUEUE_SIZE;
  if (next == queue->front) {
    return -1;
  }
  queue->chunkRefArray[queue->rear] = chunk;
  queue->rear = next;
  return 0;
}

struct chunk *dequeueChunk(struct chunk_queue *queue) {
  if (queue->front == queue->rear) {
    return NULL;
  }
  struct chunk *result = queue->chunkRefArray[queue->front];
  queue->front = (queue->front + 1) % CHUNK_QUEUE_SIZE;
  return result;
}

voxel * getVoxelAddr(struct world w, int x, int y, int z, struct chunk ** outChunk, struct region ** outRegion) {
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
    if (outRegion) *outRegion = checkRegion;
    struct chunk * checkChunk = checkRegion->chunks;
    while (checkChunk) {
      if (chunkPosInRegion.x != checkChunk->x || chunkPosInRegion.y != checkChunk->y || chunkPosInRegion.z != checkChunk->z) {
        checkChunk = checkChunk->next;
        continue;
      }
      unsigned index = (blockPosInChunk.z * CHUNK_SIZE * CHUNK_SIZE) + (blockPosInChunk.y * CHUNK_SIZE) + blockPosInChunk.x;
      if (outChunk) *outChunk = checkChunk;
      if (!checkChunk->voxels) 
        return NULL;
      return checkChunk->voxels+index;
    }
    return 0;
  }
  return 0;
}

voxel getVoxelInWorld(struct world w, int x, int y, int z) {
  voxel * loc = getVoxelAddr(w,x,y,z,0,0);
  if (!loc) return 0;
  return * loc;
}

voxel setVoxelInWorld(struct world * w, int x, int y, int z, voxel v, struct chunk_queue * meshing_queue) {
  struct chunk * chunk = NULL;
  struct region * r = NULL;
  voxel * loc = getVoxelAddr(*w,x,y,z,&chunk,&r);
  if (!loc) {
    if (!r) return 0;
    struct chunk * newEmptyChunk = calloc(1, sizeof(struct chunk));
    
    newEmptyChunk->voxels = calloc(CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE,sizeof(voxel));
    newEmptyChunk->parentRegion = r;
    newEmptyChunk->parentWorld = w;
    newEmptyChunk->x = mod((int)floor((double)x / CHUNK_SIZE) , REGION_SIZE);
    newEmptyChunk->y = mod((int)floor((double)y / CHUNK_SIZE) , REGION_SIZE);
    newEmptyChunk->z = mod((int)floor((double)z / CHUNK_SIZE) , REGION_SIZE);
    
    newEmptyChunk->next = r->chunks;
    r->chunks = newEmptyChunk;
    r = NULL, chunk = NULL;

    loc = getVoxelAddr(*w,x,y,z,&chunk,&r);
    if (!loc) return 0;
  }
    *loc = v;
    if (!chunk->needsRemesh)
      if (!chunk->inMeshQueue)
        if (enqueueChunk(meshing_queue, chunk))
          chunk->inMeshQueue = 1;;//voxelMeshData(chunk, &w);
    chunk->needsRemesh = 1;
    r->hasBeenModified = true;

    return 1;
  }



/* get the value of a block inside a chunk using chunk-relative
 * coordinates for coordinates outside of the chunk, the world
 * gets searched, which is a slightly more expensive process.
 */
voxel getVoxelInChunk(struct chunk * chunk, int x, int y, int z) {
  if (x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE || y >= CHUNK_SIZE || z >= CHUNK_SIZE) {
    ivec3 world_pos = {
      x + chunk->x * CHUNK_SIZE + ((struct region *)chunk->parentRegion)->x*REGION_SIZE*CHUNK_SIZE ,
      y + chunk->y * CHUNK_SIZE + ((struct region *)chunk->parentRegion)->y*REGION_SIZE*CHUNK_SIZE,
      z + chunk->z * CHUNK_SIZE + ((struct region *)chunk->parentRegion)->z*REGION_SIZE*CHUNK_SIZE};
    return getVoxelInWorld(*(struct world*)chunk->parentWorld, world_pos.x, world_pos.y, world_pos.z);
  }
  unsigned index = (z * CHUNK_SIZE * CHUNK_SIZE) + (y * CHUNK_SIZE) + x;
  return chunk->voxels[index];
}

unsigned voxelMeshData(struct chunk * chunk, struct world * world) {
  if (chunk->voxels == NULL) return 0;
  struct vertex { unsigned char x, y, z, w, vxl; };
  if (!chunk->vao) {
    glGenVertexArrays(1, &chunk->vao);
    glGenBuffers(1, &chunk->vbo);
    glGenBuffers(1, &chunk->ebo);
  }
  
  if (!chunk->vao_transparent) {
    glGenVertexArrays(1, &chunk->vao_transparent);
    glGenBuffers(1, &chunk->vbo_transparent);
    glGenBuffers(1, &chunk->ebo_transparent);
  }
  
  // TODO: this is definitely not the right max size
  // TODO: should DEFINITELY be static
  struct vertex * v = calloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 12, sizeof(struct vertex)); // can be static maybe?
  unsigned int * i  = calloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 18, sizeof(unsigned));

  static struct vertex * vertices_transparent = 0;
  static unsigned int * indices_transparent = 0;
  if (!vertices_transparent) {
    indices_transparent = calloc(CHUNK_SIZE * CHUNK_SIZE * 18, sizeof(unsigned));
    vertices_transparent = calloc(CHUNK_SIZE * CHUNK_SIZE * 18, sizeof(struct vertex));
  }
  
  unsigned char x, y, z;
  unsigned numVertices = 0, numVerticesTransparent = 0;
  unsigned numIndices = 0, numIndicesTransparent = 0;
 
  for (x = 0;x < CHUNK_SIZE; x++) {
    for (y = 0; y < CHUNK_SIZE; y++) {
      for (z = 0; z < CHUNK_SIZE; z++) {
        voxel voxel_current = getVoxelInChunk(chunk, x, y, z);
        if (!voxel_current) continue; 

        voxel voxel_east  = getVoxelInChunk(chunk, x+1, y  , z );
        voxel voxel_west  = getVoxelInChunk(chunk, x-1, y  , z );
        voxel voxel_above = getVoxelInChunk(chunk, x  , y+1, z );
        voxel voxel_below = getVoxelInChunk(chunk, x  , y-1, z );
        voxel voxel_south = getVoxelInChunk(chunk, x  , y  , z+1 );
        voxel voxel_north = getVoxelInChunk(chunk, x  , y  , z-1 );
        
        if (!voxel_east || (voxel_east == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z  , (0 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z  , (0 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z+1, (0 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z+1, (0 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x+1, y  , z  , (0 * 6) + 0, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z  , (0 * 6) + 1, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y  , z+1, (0 * 6) + 2, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z+1, (0 * 6) + 3, voxel_current }; numVertices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
          }
        }
        
        if (!voxel_west || (voxel_west == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z  , (1 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z  , (1 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z+1, (1 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z+1, (1 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x, y  , z  , (1 * 6) + 0, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x, y+1, z  , (1 * 6) + 1, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x, y  , z+1, (1 * 6) + 2, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x, y+1, z+1, (1 * 6) + 3, voxel_current }; numVertices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
          }  
        }
        
        if (!voxel_above || (voxel_above == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z  , (2 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z+1, (2 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z  , (2 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z+1, (2 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x  , y+1, z  , (2 * 6) + 0, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x  , y+1, z+1, (2 * 6) + 1, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z  , (2 * 6) + 2, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z+1, (2 * 6) + 3, voxel_current }; numVertices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
          }
        }
        
        if (!voxel_below || (voxel_below == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z  , (3 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z  , (3 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z+1, (3 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z+1, (3 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x  , y, z  , (3 * 6) + 0, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y, z  , (3 * 6) + 1, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x  , y, z+1, (3 * 6) + 2, voxel_current }; numVertices++;
            v[numVertices] = (struct vertex){x+1, y, z+1, (3 * 6) + 3, voxel_current }; numVertices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
          }
        }
        
        if (!voxel_south || (voxel_south == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z+1, (4 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z+1, (4 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z+1, (4 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z+1, (4 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x  , y  , z+1, (4 * 6) + 0, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x  , y+1, z+1, (4 * 6) + 1, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x+1, y  , z+1, (4 * 6) + 2, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z+1, (4 * 6) + 3, voxel_current}; numVertices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
          }
        }
        
        if (!voxel_north || (voxel_north == 6 && voxel_current != 6)) {
          if (voxel_current == 6) {
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y  , z  , (5 * 6) + 0, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x  , y+1, z  , (5 * 6) + 1, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y  , z  , (5 * 6) + 2, voxel_current }; numVerticesTransparent++;
            vertices_transparent[numVerticesTransparent] = (struct vertex){x+1, y+1, z  , (5 * 6) + 3, voxel_current }; numVerticesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-4; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-1; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-2; numIndicesTransparent++;
            indices_transparent[numIndicesTransparent] = numVerticesTransparent-3; numIndicesTransparent++;
          } else {
            v[numVertices] = (struct vertex){x  , y  , z  , (5 * 6) + 0, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x  , y+1, z  , (5 * 6) + 1, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x+1, y  , z  , (5 * 6) + 2, voxel_current}; numVertices++;
            v[numVertices] = (struct vertex){x+1, y+1, z  , (5 * 6) + 3, voxel_current}; numVertices++;
            i[numIndices] = numVertices-4; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-1; numIndices++;
            i[numIndices] = numVertices-2; numIndices++;
            i[numIndices] = numVertices-3; numIndices++;
          }
        }
      }
    }
  }
  
  glBindVertexArray(chunk->vao);    
  glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo);
  glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(struct vertex), v, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), i, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(struct vertex), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)3);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)4);
  glBindVertexArray(0);
  
  glBindVertexArray(chunk->vao_transparent);    
  glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_transparent);
  glBufferData(GL_ARRAY_BUFFER, numVerticesTransparent * sizeof(struct vertex), vertices_transparent, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ebo_transparent);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndicesTransparent * sizeof(unsigned int), indices_transparent, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_BYTE, GL_FALSE, sizeof(struct vertex), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)3);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(struct vertex), (void*)4);
  glBindVertexArray(0);
  free(v);
  free(i);
  chunk->icount = numIndices;
  chunk->icount_transparent = numIndicesTransparent;
  return numIndices;
}

/* save "worldinfo" and region files.
   other data like player data must
   be saved separately.
*/ 
int saveWorld(struct world * w) {
  mkdir(w->name, 0777);
  
  char filename[128];
  sprintf(filename, "%s/worldinfo", w->name);
  FILE * fptr = fopen(filename,"wb");
  const char dummy = 1; // world version
  fwrite(&dummy, 1, 1, fptr);
  fclose(fptr);

  struct region * r = w->loadedRegions;
  while (r) {
    if (r->hasBeenModified) {
      char filename[256];
      sprintf(filename, "%s/r%2d_%2d_%2d", w->name, r->x, r->y, r->z);
      FILE * fptr = fopen(filename,"wb");
      struct chunk * c = r->chunks;
      while (c) {
        if (!c->voxels) { c = c->next; continue; }
        struct chunk_file chunk_file;
        chunk_file.x = c->x;
        chunk_file.y = c->y;
        chunk_file.z = c->z;
        fwrite(&chunk_file, sizeof(struct chunk_file), 1, fptr);
        fwrite(c->voxels, sizeof(voxel), CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, fptr );
        c = c->next;
      }
      r->hasBeenModified = 0;
      fclose(fptr);
    }
    r = r->next;
  }
}

int loadWorld(struct world *w) {
    DIR *dir = opendir(w->name);
    char worldinfo_fnbuff[64];
    sprintf(worldinfo_fnbuff, "%s/worldinfo", w->name);
    FILE * fptr = fopen(worldinfo_fnbuff, "rb");
    unsigned char version;
    fread(&version, 1, 1, fptr);
    fclose(fptr);
    if (!dir) exit(678);
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
        if (!fptr) exit(679);
        struct region *region = calloc(1, sizeof(struct region));
        region->x = rx;
        region->y = ry;
        region->z = rz;
        region->hasBeenModified = version == 0 ? 1 : 0;
        region->next = w->loadedRegions;
        w->loadedRegions = region;
        while (1) {
            if (version == 0) {
              struct legacy_chunk legacy_chunk;
              size_t n = fread(&legacy_chunk, sizeof(struct legacy_chunk), 1, fptr);
              if (n != 1) 
                break;
              struct chunk *chunk = malloc(sizeof(struct chunk));
              chunk->x = legacy_chunk.x;
              chunk->y = legacy_chunk.y;
              chunk->z = legacy_chunk.z;
              chunk->parentWorld = w;
              chunk->parentRegion = region;
              chunk->voxels = malloc(sizeof(voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
              fread(chunk->voxels, sizeof(voxel),CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, fptr);
              chunk->next = region->chunks;
              region->chunks = chunk;
            } else {
              struct chunk_file chunk_file;
              size_t n = fread(&chunk_file, sizeof(struct chunk_file), 1, fptr);
              if (n != 1) 
                break;
              struct chunk *chunk = malloc(sizeof(struct chunk));
              chunk->x = chunk_file.x;
              chunk->y = chunk_file.y;
              chunk->z = chunk_file.z;
              chunk->parentWorld = w;
              chunk->parentRegion = region;
              chunk->voxels = malloc(sizeof(voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
              fread(chunk->voxels, sizeof(voxel),CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, fptr);
              int chunkIsEmpty = 1;
              for(int i = 0; i < CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; i++) {
                  if (chunk->voxels[i]) {chunkIsEmpty = 0; break;}
              }
              if (chunkIsEmpty) {
                free(chunk->voxels);
                chunk->voxels = NULL;
                free(chunk);
                continue;
              }
              chunk->next = region->chunks;
              region->chunks = chunk;
            }
        }
        fclose(fptr);
    }
    closedir(dir);
    return 0;
}

void freeWorld(struct world * w) {
  struct region * r = w->loadedRegions;
  while (r) {
    struct chunk * chunk = r->chunks;
    while (chunk) {
      free(chunk->voxels);

      glDeleteVertexArrays(1, &chunk->vao);
      glDeleteBuffers(1, &chunk->vbo);
      glDeleteBuffers(1, &chunk->ebo);
      glDeleteVertexArrays(1, &chunk->vao_transparent);
      glDeleteBuffers(1, &chunk->vbo_transparent);
      glDeleteBuffers(1, &chunk->ebo_transparent);
      
      struct chunk * chunkFree = chunk;
      chunk = chunk->next;
      free(chunkFree);
    }
    struct region * rFree = r;
    r = r->next;
    free(rFree);
  }
  w->loadedRegions = NULL;
}

struct chunk * generChunkHeightmap(struct world * w, int x, int y, int z) {

  static int * noiseMap2D = 0;
  if (!noiseMap2D) {
    noiseMap2D = malloc(sizeof(int) * CHUNK_SIZE * CHUNK_SIZE);
  }

  for (int bx = 0; bx < CHUNK_SIZE; bx++) {
    for (int bz = 0; bz < CHUNK_SIZE; bz++) {
      float perlin2d_octaves(float x, float y, int octaves, float persistence);
      float perlin2d(float x, float y);
      const float xzScale = 0.0015; // 0.02
      const float yScale = 480; // 80
      int octaves = 6;
      float persp = 0.5;
      float perlinValue = perlin2d_octaves((x * CHUNK_SIZE + bx) * xzScale, (z * CHUNK_SIZE + bz) * xzScale, octaves, persp);
      noiseMap2D[(bz * CHUNK_SIZE) + bx] = yScale * perlinValue;
    }
  }
  
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
    .parentWorld = w,
    .parentRegion = checkRegion,
    .next = checkRegion->chunks,
    .voxels = calloc(sizeof(voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE, 1)
  };
  unsigned max = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
  for (int bx = 0; bx < CHUNK_SIZE; bx++) {
    for (int by = 0; by < CHUNK_SIZE; by++) {
      for (int bz = 0; bz < CHUNK_SIZE; bz++) {
        int surfaceDepth = noiseMap2D[bz * CHUNK_SIZE + bx] - (by + y * CHUNK_SIZE);
        voxel v;

        if (surfaceDepth < 0) {
          v = y * CHUNK_SIZE + by >= 0 ? 0 : 0;// 0 : 6
          
        } else if (surfaceDepth < 3) {
          v = 1;
        } else {
          v = 2;
        }
        newChunk->voxels[(bz * CHUNK_SIZE * CHUNK_SIZE) + (by * CHUNK_SIZE) + bx] = v;
      }
    }
  }
  checkRegion->chunks = newChunk;
  return newChunk;
}

void * createWorld(void * arg) {
  struct world * w = (struct world *) arg;
  w->loadedRegions = NULL;
  for (int x = 0; x < (w->size_h); x++) {
    for (int y = 0; y < (w->size_v); y++) {
      for (int z = 0; z < (w->size_h); z++) {
        generChunkHeightmap(w, x - (w->size_h / 2), y - (w->size_v / 2), z - (w->size_h / 2));
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
// is supplied with projection and view matrices and texture


void drawAllChunks(struct world * w, struct chunk_queue * meshing_queue) {
  glUseProgram(w->shaderProgram);
  
  unsigned modelUniformLoc    = glGetUniformLocation(w->shaderProgram, "model");
  unsigned chunkPosUniformLoc = glGetUniformLocation(w->shaderProgram, "chunkPos");
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, w->tex);
  
  struct region * r = w->loadedRegions;
  struct chunk * c = NULL;
  ivec3 chunkPos;
  mat4 model;
  /* draw solid world */
  while (r) {
    c = r->chunks;
    while (c) {
      if (c->voxels == NULL) {c = c->next; continue;}
      chunkPos = (ivec3){r->x * REGION_SIZE + c->x , r->y * REGION_SIZE + c->y, r->z * REGION_SIZE + c->z};
      mat4Translate(model, chunkPos.x * CHUNK_SIZE, chunkPos.y * CHUNK_SIZE, chunkPos.z * CHUNK_SIZE);
      // TODO : eliminate one of these uniforms because its totally redundant
      glUseProgram(w->shaderProgram);
      glUniformMatrix4fv(modelUniformLoc, 1, GL_FALSE, model);
      glUniform3f(chunkPosUniformLoc, chunkPos.x * CHUNK_SIZE, chunkPos.y * CHUNK_SIZE, chunkPos.z * CHUNK_SIZE);
      if (!c->vao) {
        if (!c->inMeshQueue)
          if (enqueueChunk(meshing_queue, c))
            c->inMeshQueue = 1;

        /* this is what id like to do but its really slow for some reason
        c = c->next;
        continue;
        */
        return; // so i do this instead
      }
      
      glBindVertexArray(c->vao);
      
      glDrawElements(GL_TRIANGLES, c->icount, GL_UNSIGNED_INT, 0);
      c = c->next;
    }
    r = r->next;
  }
  transparent:
  /* draw transparent world */
  r = w->loadedRegions;
  while (r) {
    c = r->chunks;
    while (c) {
      if (c->voxels == NULL) {c = c->next; continue;};
      if (!c->vao) {c = c->next; continue;};
      chunkPos = (ivec3){r->x * REGION_SIZE + c->x , r->y * REGION_SIZE + c->y, r->z * REGION_SIZE + c->z};
      mat4Translate(model, chunkPos.x * CHUNK_SIZE, chunkPos.y * CHUNK_SIZE, chunkPos.z * CHUNK_SIZE);
      // TODO : eliminate one of these uniforms because its totally redundant
      glUseProgram(w->shaderProgram);
      glUniformMatrix4fv(modelUniformLoc, 1, GL_FALSE, model);
      glUniform3f(chunkPosUniformLoc, chunkPos.x * CHUNK_SIZE, chunkPos.y * CHUNK_SIZE, chunkPos.z * CHUNK_SIZE);
      
      glBindVertexArray(c->vao_transparent);
      
      glDrawElements(GL_TRIANGLES, c->icount_transparent, GL_UNSIGNED_INT, 0);
      c = c->next;
    }
    r = r->next;
  }

  // 

  
}