//
// Created by jack lewis on 11/15/25.
//

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include <GLFW/glfw3.h>

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
    struct world world;
    int onGround;
} player_t;

void processPlayerMovement(player_t *, double);



typedef char byte_t;

struct Vertex {
    byte_t x, y, z, i, j, k, u, v;
};

GLuint createMesh(struct Vertex *, unsigned *, unsigned, unsigned);

unsigned shaderProgram(char * vertexPath, char * fragmentPath);
typedef enum {SERVER_INTERNAL, SERVER_EXTERNAL} servertype;
typedef unsigned pixel_t;
typedef long player_id;
struct blockAtlas {
    pixel_t * pixels;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    player_id * textureOwners;
};
struct playerdata {
    double   wX, wY, wZ;
    int      rX, rY, rZ;
    unsigned cX, cY, cZ;
    unsigned bX, bY, bZ;
    
};
struct server {
    servertype serverType;
    struct world * world;
    struct blockAtlas blockAtlas;
    struct playerdata * players;
};
struct client {
    unsigned long blockType;
    int isConnectedToServer;
    struct world world;
    servertype serverType;
    unsigned shaders[5];
    player_t player;
};

typedef struct {
    int * shaders;
    size_t nShaders;
} client;

GLuint createCubeMesh();

void beginClientLoop(GLFWwindow *, struct client *);
#define WIDTH  1280
#define HEIGHT 960
#define WINDOW_NAME "3D Rendering"

#endif //MAIN_H
