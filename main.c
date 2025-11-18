#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>

#include "tgui/tgui.h"
#include "main.h"
#include "gui.h"


static uint32_t pixels[WIDTH * HEIGHT];

static GLuint textureAtlas;

static GLuint vao;

unsigned char blockType = 1; // global



int fb_width, fb_height;
int guiEnabled = 0;
struct world world;
player_t player;

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

    /* compile + link shaders */
    GLuint prog =               shaderProgram("shaders/vertex.glsl","shaders/fragment.glsl");
    GLuint worldshader =        shaderProgram("shaders/world.vert", "shaders/world.frag");
    GLuint crosshairShader =    shaderProgram("shaders/crosshair.vert", "shaders/crosshair.frag");

    /* input */
    inputInit(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    
    /* create texture */
    glGenTextures(1, &textureAtlas);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureAtlas);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    unsigned framecount = 0;

    // matrices
    
    mat4 proj; mat4Proj(proj, 1.5707963f, (float)WIDTH / (float)HEIGHT, 0.1f, 2048.0f);
    glUseProgram(prog);        glUniformMatrix4fv(glGetUniformLocation(prog,"projection"), 1, GL_FALSE, proj);
    glUseProgram(worldshader); glUniformMatrix4fv(glGetUniformLocation(worldshader,"projection"), 1, GL_FALSE, proj);
    
    glEnable(GL_DEPTH_TEST);

    // create things
    world = initTestWorld();
    world.shaderProgram = worldshader;
    world.tex = textureAtlas;
    
    thing_t blockHighlight = {
        .prog = prog,
        .tex = textureAtlas,
        .vao = createMesh(vertices, indices, vSize, iSize),
        .n = 36
    };
    mat4Identity(blockHighlight.transform);
    thing_t crosshair = {
        .prog = crosshairShader,
        .tex = textureAtlas,
        .vao = createMesh(vertices, indices, vSize, iSize),
        .n = 6
    };
    mat4Scale(crosshair.transform, 0.01, 0.01, 0.01);
    
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

    double lastTime = glfwGetTime();

    double frameBeginTime;
    double frameEndTime;
    double mspf = 0;
    
    while (!glfwWindowShouldClose(window)) {
        
        frameBeginTime = glfwGetTime();
        
        gInput.mouseDeltaX = 0.0;
        gInput.mouseDeltaY = 0.0;
        gInput.scrollY = 0.0;
        glfwPollEvents();
        double now = glfwGetTime();
        float delta = (float)(now - lastTime);
        lastTime = now;
        framecount++;
        int fps;
        
        
        
        /* entity processing */
        
        if (!guiEnabled) processPlayerMovement(&player, delta);
        
        /* block highlight */
        
        ivec3 hit;
        int hitfound = voxelRaycastHit(world,(fvec3){ player.pos.x , player.pos.y + player.cameraHeight, player.pos.z}, player.yaw - M_PI * 0.5, player.pitch * -1, 5, &hit);
        //("break8\n");
        if (hitfound) {
            mat4Translate(blockHighlight.transform, hit.x+0.5, hit.y+0.5, hit.z+0.5);
        } else {
            mat4Scale(blockHighlight.transform, 0, 0, 0);
        }
        
        /* debug info text */
        
        char debugBuffer[80];
        sprintf(debugBuffer, "%4d,%4d,%4d", (int)player.pos.x, (int)player.pos.y, (int)player.pos.z);
        
        /* gui */
        
        if (gInput.keys[GLFW_KEY_ESCAPE]) { 
            //glfwSetWindowShouldClose(window, 1);
            guiEnabled = 1; glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (gInput.keys[GLFW_KEY_ENTER]) {
            guiEnabled = 0; glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        
        /* developer menu */
        
        


        /* draw */
        
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

    }
    /* cleanup */
    glDeleteTextures(1, &textureAtlas);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(prog);
    glDeleteProgram(worldshader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
