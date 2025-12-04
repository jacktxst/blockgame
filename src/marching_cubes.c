//
// Created by jack lewis on 11/28/25.
//

#include "voxels.h"

static inline int clamp(int clamp_arg, int min, int max) {
  return clamp_arg<min?min:(clamp_arg>max?max:clamp_arg);
}

static inline int voxel_index(ivec3 v) {
  static const int CHUNK_LAYER_SIZE = CHUNK_SIZE * CHUNK_SIZE;
  return v.x + (v.y * CHUNK_SIZE) + (v.z * CHUNK_LAYER_SIZE);
}

ivec3 clamp_vec3(ivec3 v, ivec3 min, ivec3 max) {
  return (ivec3){clamp(v.x,min.x,max.x),clamp(v.y,min.y,max.y),clamp(v.z,min.z,max.z)}; 
}

void marchingCubes(unsigned char * v, unsigned * v_size, unsigned char * e, unsigned * e_size, unsigned char * voxels) {
  unsigned x, y, z = 0;
  static const unsigned CHUNK_LAYER_SIZE = CHUNK_SIZE * CHUNK_SIZE;
  static const ivec3 min = {0,0,0}, max = {CHUNK_SIZE-1, CHUNK_SIZE-1, CHUNK_SIZE-1};
  static const unsigned int marching_cubes_cases[256] = {
    // each element is an index into triangle_sets
  };
  static const unsigned char * triangle_sets = {
    // one triangle set entry: [0] = size of the entry [1..] = vertex data
  };
  while(1){
    unsigned char cube_values[8] = {
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))],
      voxels[voxel_index(clamp_vec3((ivec3){x  ,y  ,z  },min,max))]
    };
    unsigned char triangle_set_id = 
        (128*cube_values[0]?1:0)+
        ( 64*cube_values[1]?1:0)+
        ( 32*cube_values[2]?1:0)+
        ( 16*cube_values[3]?1:0)+
        (  8*cube_values[4]?1:0)+
        (  4*cube_values[5]?1:0)+
        (  2*cube_values[6]?1:0)+
        (  1*cube_values[7]?1:0);
    unsigned char const * triangle_set = triangle_sets[triangle_set_id];
    int n_tris = triangle_set[0];
    for(int i=1;i<n_tris*9+1;i++){
      v[*v_size] = triangle_set[i];
      (*v_size)++;
    }
  }
}