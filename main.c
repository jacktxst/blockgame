#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "main.h"
#include "gui.h"

int fb_width, fb_height;

typedef enum {SERVER_INTERNAL, SERVER_EXTERNAL} servertype;
struct server { servertype serverType; struct world * world; };
struct client { struct world * world; servertype serverType; };

int main(void) {
    /* lib init */
    if (!glfwInit()) { fprintf(stderr, "failed to init glfw\n"); return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_NAME, NULL, NULL);
    if (!window) { fprintf(stderr, "failed to create window\n"); glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { fprintf(stderr, "failed to init glew\n"); return -1; }

    /* setup input */
    inputInit(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    
    /* main menu */
  while (!glfwWindowShouldClose(window)) {
    gInput.mouseDeltaX = 0.0;
    gInput.mouseDeltaY = 0.0;
    gInput.scrollY = 0.0;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    glViewport(0, 0, fb_width, fb_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glfwPollEvents();
      
    struct guictx ctx = {
      .scrw = fb_width,
      .scrh = fb_height,
      .mx = gInput.mouseX * 2 ,
      .my = gInput.mouseY * 2 ,
      .mb = gInput.mouseButtons[GLFW_MOUSE_BUTTON_LEFT],
      .typedChar =  gInput.typedChar,
      .sclX = 8,
      .sclY = 16,
      .keys = gInput.keys };
    //drawText("test",0,0,fb_width,fb_height,8,16);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);

    static int showConnectMenu = 0;
    static int showCreateWorldMenu = 0;
    static struct world world = {0};
    static struct server internalServer = {0};
      static int worldIsBeingCreated = 0;
      static int showMainMenu = 1;
      if (showMainMenu) {
          if (button_once(&ctx, 60, 200, "create world", "tooltip")) {
              showCreateWorldMenu = !showCreateWorldMenu;
              showConnectMenu = 0;
          }
          button_once(&ctx, 60, 300, "load world", "tooltip");
          if (button_once(&ctx, 60, 400, "join world", "tooltip")) {
              showCreateWorldMenu = 0;
              showConnectMenu = !showConnectMenu;
          }
          
      }
    if (showCreateWorldMenu) {
        static textfield_t worldNameTextfield = {0};
        textfield(&ctx, &worldNameTextfield, 650, 200, "world name", 10, "");
        
        static textfield_t worldHeightTextfield = {0};
        textfield(&ctx, &worldHeightTextfield, 650, 300, "", 3, "");
        drawText("world height (chunks)",800,300,fb_width,fb_height,8,16);
        
        
        static textfield_t worldSizeTextfield = {0};
        textfield(&ctx, &worldSizeTextfield, 650, 400, "", 3, "");
        drawText("world size (chunks)",800,400,fb_width,fb_height,8,16);
        
        int pregenerateWorld;
        checkbox(&ctx, 650, 500, &pregenerateWorld, "");
        drawText("pre-generate world",800,500,fb_width,fb_height,8,16);

        if ( button_once(&ctx, 650, 600, "create", "ok")) {
            // world creation process
            
            
            void initServerThread(struct server *);
            initServerThread(&internalServer);
            
            struct client client = {
                .world = &world,
                .serverType = SERVER_INTERNAL,
            };
            showCreateWorldMenu = 0;
            showMainMenu = 0;
            worldIsBeingCreated = 1;
            
        }
        
    }

    if (worldIsBeingCreated) {
        
        // if (serverIsReadyToJoin) {
        //   worldIsBeingCreated = 0;
        //   beginClientLoop(&client);
        //}
        
        char * loadingMessage = "Loading world...";
        drawText(loadingMessage, 650, 600, fb_width,fb_height,8,16);
        
    }
      
    if (showConnectMenu) {
        static textfield_t ipAddrTextfield = {0};
        textfield(&ctx, &ipAddrTextfield, 650, 400, "ip", 10, "ok");

        static textfield_t portTextfield = {0};
        textfield(&ctx, &portTextfield, 650, 500, "port", 10, "ok");

        button_once(&ctx, 650, 600, "connect", "ok");
        
    }
        


    gInput.typedChar = 0;
    glfwSwapBuffers(window);
  }
    
    
    /* cleanup */
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}


typedef struct {
    int * shaders;
    size_t nShaders;
} client;

void initClient(client * c, GLFWwindow * window) {
    /*
    int onKey(int, int);
    player = (player_t){
        .cameraHeight = 1.5,
        .radius = 0.25,
        .height = 1.9,
        .speed = 6,
        .gravity = 12,
        .jump=7,
        .move = NORMAL,
        .onKey = onKey
    };
    thing_t blockHighlight = {
        .prog = c->shaders[0],
        .tex = 0,
        .vao = createMesh(vertices, indices, vSize, iSize),
        .n = 36
    };
    mat4Identity(blockHighlight.transform);
    thing_t crosshair = {
        .prog = c->shaders[1],
        .tex = 0,
        .vao = createMesh(vertices, indices, vSize, iSize),
        .n = 6
    };
    mat4Scale(crosshair.transform, 0.01, 0.01, 0.01);
    */
    void beginClientLoop(GLFWwindow * window);
    beginClientLoop(window);
}

void beginClientLoop(GLFWwindow * window) {
    
    double currentTime = 0;
    double begin = glfwGetTime();
    double lastTime = 0;
    
    while (!glfwWindowShouldClose(window)) {
        
        lastTime = currentTime;
        currentTime = glfwGetTime() - begin;
        double delta = currentTime - lastTime;
        
        gInput.mouseDeltaX = 0.0;
        gInput.mouseDeltaY = 0.0;
        gInput.scrollY = 0.0;
        glfwPollEvents();
        
        /* entity processing */
        
        //if (!guiEnabled) processPlayerMovement(&player, delta);
        
        /* block highlight 
        
        ivec3 hit;
        int hitfound = voxelRaycastHit(world,(fvec3){ player.pos.x , player.pos.y + player.cameraHeight, player.pos.z}, player.yaw - M_PI * 0.5, player.pitch * -1, 5, &hit);
        //("break8\n");
        if (hitfound) {
            mat4Translate(blockHighlight.transform, hit.x+0.5, hit.y+0.5, hit.z+0.5);
        } else {
            mat4Scale(blockHighlight.transform, 0, 0, 0);
        }
        
         debug info text 
        
        char debugBuffer[80];
        sprintf(debugBuffer, "%4d,%4d,%4d", (int)player.pos.x, (int)player.pos.y, (int)player.pos.z);
        
        /* gui 
        
        if (gInput.keys[GLFW_KEY_ESCAPE]) { 
            //glfwSetWindowShouldClose(window, 1);
            guiEnabled = 1; glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (gInput.keys[GLFW_KEY_ENTER]) {
            guiEnabled = 0; glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        
         developer menu */
        
        


        /* draw 
        
        mat4 m1;            mat4RotateX(m1,player.pitch);
        mat4 m2;            mat4RotateY(m2,player.yaw);
        mat4 m3;            mat4Multiply (m3, m1, m2);
        mat4 m4;            mat4Translate(m4, -player.pos.x, -(player.pos.y+player.cameraHeight+0.01*sin(now*7)), -player.pos.z);
        mat4 viewMatrix;    mat4Multiply (viewMatrix,m3, m4);
        glUseProgram(prog); glUniformMatrix4fv(glGetUniformLocation(prog,"view"), 1, GL_FALSE, viewMatrix);
        glUseProgram(worldshader); glUniformMatrix4fv(glGetUniformLocation(worldshader,"view"), 1, GL_FALSE, viewMatrix);
        unsigned tex0UniformLoc = glGetUniformLocation(worldshader, "tex0");
        glUniform1i(tex0UniformLoc, 0);
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        static int wireframe = 0;

        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
        void drawWorld(struct world w, fvec3 pos, int dh, int dv);
        static unsigned color = 0xFFFFFFFF;
        float a = ((color >> 24) & 0xFF) / 255.0;
        float r = ((color >> 16) & 0xFF) / 255.0;
        float g = ((color >> 8)  & 0xFF) / 255.0;
        float b = (color & 0xFFu) / 255.0;
        glUniform3f(glGetUniformLocation(worldshader, "color"), b, g, r);
        //assumes that viewmodel has been set
        
        drawWorld(world, player.pos, RENDER_DISTANCE, RENDER_DISTANCE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        drawThing(&blockHighlight);
        
        glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);


        struct guictx ctx = {
            .scrw = fb_width,
            .scrh = fb_height,
            .mx = guiEnabled ? gInput.mouseX*2 : 0,
            .my = guiEnabled ? gInput.mouseY*2 : 0,
            .mb = gInput.mouseButtons[GLFW_MOUSE_BUTTON_LEFT],
            .typedChar = guiEnabled ? gInput.typedChar : 0,
            .sclX = 8,
            .sclY = 16,
            .keys = gInput.keys
        };
        slider(&ctx, 32,300, &player.speed, 0.0, 200.0, "speed");
        slider(&ctx, 32,400, &player.gravity, 0.0, 20.0, "gravity");
        slider(&ctx, 32,500, &player.jump, 0.0, 20.0, "jump");
        static int flycheckbox = 1;
        checkbox(&ctx,32,600, &flycheckbox, "flycheckbox");
        player.move = flycheckbox ? FLY : NORMAL;
        checkbox(&ctx,32,700, &wireframe, "wireframe");
        
        colorpicker(&ctx, 400, 800, &color, "test");
        
        glUseProgram(worldshader);
        glUniform1i(glGetUniformLocation(worldshader, "tex"), 0);

        //struct {GLuint texture = 0; unsigned * data = 0; unsigned nLayers = 3;} atlas;
        static GLuint atlasTexture = 0;
        static unsigned * dataPtr = 0;
        static unsigned nLayers = 3;
        atlasedit(&ctx, 32, 800, 16, 16, &nLayers, blockType, color, &dataPtr, &atlasTexture );
        world.tex = atlasTexture;
        
        static char * fieldbuff = 0;
        static unsigned cursor = 0;
        static int txfieldfocused = 0;
        
        if (button_once(&ctx, 32, 1080, "+","increment block type")) {blockType ++;};  
        if (button_once(&ctx, 128, 1080, "-","decrement block type")) blockType--;
        if (button_once(&ctx, 256, 1080, "sav","save texture atlas")) {
            FILE * fptr = fopen("atlas.texatlas", "wb");
            fputc(nLayers, fptr);
            fwrite(dataPtr, sizeof(unsigned), 16 * 16 * nLayers, fptr);
            fclose(fptr);
        };
        if (button_once(&ctx, 512, 1080, "lod","load texture atlas")) {
            FILE * fptr = fopen("atlas.texatlas", "rb");
            if (fptr) {
                fread(&nLayers, sizeof(unsigned char), 1, fptr);
                dataPtr = realloc(dataPtr, nLayers * 16 * 16 * sizeof(unsigned));
                fread(dataPtr, sizeof(unsigned), 16 * 16 * nLayers, fptr);
                glBindTexture(GL_TEXTURE_2D_ARRAY, atlasTexture);
                // must reallocate entire array
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
                             GL_RGBA8, 16, 16, nLayers,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, dataPtr);
                fclose(fptr);
            }
        };
        {
            char buffer[5];
            sprintf(buffer, "%d",blockType);
            drawText(buffer, 32, 1200, fb_width, fb_height, 8, 16);
        }
        
        
        textfield(&ctx, 128, 1200, "hiiii", &fieldbuff, &cursor, &txfieldfocused, "example");
        char buff[80]; sprintf(buff, "xyz %5d %5d %5d fps", (int)player.pos.x, (int)player.pos.y, (int)player.pos.z);
        drawText(buff,32,0, fb_width, fb_height, 8, 16);
        char buff2[30];
        sprintf(buff2, "vel %5d %5d %5d", (int)player.vel.x, (int)player.vel.y, (int)player.vel.z);
        drawText(buff2,32,128, fb_width, fb_height, 8, 16);
        gInput.typedChar = 0;


        glEnable(GL_DEPTH_TEST);
        drawThing(&crosshair);
        glfwSwapBuffers(window);
        */
        

    }
}