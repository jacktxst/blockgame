//
// Created by jack lewis on 11/15/25.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

static const char vertices [] = {
    // front face (+Z)
    -64, -64,  64, 0,0,0,   0,   0,
     64, -64,  64, 0,0,0, 255,   0,
     64,  64,  64, 0,0,0, 255, 255,
    -64,  64,  64, 0,0,0,   0, 255,
    // back face (-Z)
     64, -64, -64, 0,0,0,   0,   0,
    -64, -64, -64, 0,0,0, 255,   0,
    -64,  64, -64, 0,0,0, 255, 255,
     64,  64, -64, 0,0,0,   0, 255,
    // left face (-X)
    -64, -64, -64, 0,0,0,   0,   0,
    -64, -64,  64, 0,0,0, 255,   0,
    -64,  64,  64, 0,0,0, 255, 255,
    -64,  64, -64, 0,0,0,   0, 255,
    // right face (+X)
     64, -64,  64, 0,0,0,   0,   0,
     64, -64, -64, 0,0,0, 255,   0,
     64,  64, -64, 0,0,0, 255, 255,
     64,  64,  64, 0,0,0,   0, 255,
    // top face (+Y)
    -64,  64,  64, 0,0,0,   0,   0,
     64,  64,  64, 0,0,0, 255,   0,
     64,  64, -64, 0,0,0, 255, 255,
    -64,  64, -64, 0,0,0,   0, 255,
    // bottom face (-Y)
    -64, -64, -64, 0,0,0,   0,   0,
     64, -64, -64, 0,0,0, 255,   0,
     64, -64,  64, 0,0,0, 255, 255,
    -64, -64,  64, 0,0,0,   0, 255,
};

static const unsigned indices [] = {
    // front
    0, 1, 2,  0, 2, 3,
    // back
    4, 5, 6,  4, 6, 7,
    // left
    8, 9,10,  8,10,11,
    // right
   12,13,14, 12,14,15,
    // top
   16,17,18, 16,18,19,
    // bottom
   20,21,22, 20,22,23
};

static const unsigned vSize = 24;
static const unsigned iSize = 36;

typedef struct InputState {
    int keys[GLFW_KEY_LAST + 1];
    int mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    double mouseX;
    double mouseY;
    double mouseDeltaX;
    double mouseDeltaY;
    double scrollY;
    int firstMouse;
    unsigned char typedChar;
} InputState;

// Global input state (for convenience)
extern InputState gInput;

// Initialize callbacks and clear state
void inputInit(GLFWwindow * window);

#ifndef INC_3DMATH_H
#define INC_3DMATH_H

//typedef float * mat4;
typedef float mat4[16];


typedef struct {int x, y, z;} ivec3;
typedef struct {int x, y;} ivec2;
typedef struct {float x, y, z;} fvec3;
typedef struct {float x, y;} fvec2;

void mat4Ortho(mat4 dest, float left, float right, float bottom, float top, float nearZ, float farZ);
void mat4Multiply(mat4 dest, mat4 a, mat4 b);
void mat4Identity(mat4);
void mat4Translate(mat4 dest, float x, float y, float z);
void mat4Scale(mat4 dest, float x, float y, float z);
void mat4Proj(mat4 dest, float fovyRadians, float aspect, float nearZ, float farZ);
void mat4RotateX(mat4 dest, float radians);
void mat4RotateY(mat4 dest, float radians);
void mat4RotateZ(mat4 dest, float radians);

#endif //INC_3DMATH_H

typedef struct thing {
    unsigned vao;
    unsigned n;
    unsigned tex;
    unsigned prog;
    unsigned count;
    mat4 transform;
} thing_t;

int drawThing(thing_t * thing);
typedef enum move {
    FLY,
    NORMAL
  } move_t;

typedef struct player {
    fvec3 pos;
    fvec3 vel;
    double yaw;
    double pitch;
    move_t move;
    double gravity;
    double radius;
    double height;
    double jump;
    double speed;
    double acceleration;
    double cameraHeight;
    int onGround;
    int (*onKey)(int,int);
} player_t;

void processPlayerMovement(player_t *, double);

#ifndef VOXELS_H
#define VOXELS_H

#define WORLD_SIZE 4
#define REGION_SIZE 16
#define CHUNK_SIZE 16
#define RENDER_DISTANCE 8
#define ANTIDENSITY 200

typedef unsigned char voxel;

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
    struct region * loadedRegions;
    GLuint tex;
};

void saveWorldFiles(struct world world);

struct world initTestWorld();
unsigned voxelMeshData(struct chunk * chunk);
int voxelRaycastHit(
    struct world world,
    fvec3 o,
    float yaw, float pitch,
    float maxDist,
    ivec3 * hit
    );
voxel getVoxelInWorld(struct world w, int x, int y, int z);

voxel setVoxelInWorld(struct world w, int x, int y, int z, voxel v);

int voxelRaycastPlace(
    struct world world,
    fvec3 o,
    float yaw, float pitch,
    float maxDist,
    ivec3 * hit
    );

#endif //VOXELS_H

typedef char byte_t;

struct Vertex {
    byte_t x, y, z, i, j, k, u, v;
};

GLuint createMesh(struct Vertex *, unsigned *, unsigned, unsigned);

unsigned shaderProgram(char * vertexPath, char * fragmentPath);

#define WIDTH  1280
#define HEIGHT 960
#define WINDOW_NAME "3D Rendering"

extern struct voxelMesh_t voxelMesh;
extern struct world world;
extern thing_t chunk1;
extern thing_t chunk2;
extern player_t player;
extern int fb_width, fb_height;

#endif //MAIN_H
