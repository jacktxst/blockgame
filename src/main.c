#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <pthread.h>


#include "main.h"
#include "gui.h"
#include "math.h"
#include "voxels.h"

int fb_width, fb_height;

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

    /* graphics pipeline setup */
    struct client client;
    client.shaders[0] = shaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    client.shaders[1] = shaderProgram("shaders/world.vert", "shaders/world.frag");
    client.shaders[2] = shaderProgram("shaders/crosshair.vert", "shaders/crosshair.frag");
    
    mat4 proj; mat4Proj(proj, 1.5707963f, (float)WIDTH / (float)HEIGHT, 0.1f, 2048.0f);
    // default shader
    glUseProgram(client.shaders[0]);
    glUniform1i(glGetUniformLocation(client.shaders[0], "tex"), 0);
    glUniformMatrix4fv(glGetUniformLocation(client.shaders[0],"projection"), 1, GL_FALSE, proj);
    // world shader
    glUseProgram(client.shaders[1]);
    glUniform1i(glGetUniformLocation(client.shaders[1], "tex0"), 0);
    glUniform1i(glGetUniformLocation(client.shaders[1], "tex1"), 1);
    glUniformMatrix4fv(glGetUniformLocation(client.shaders[1],"projection"), 1, GL_FALSE, proj);
    
    /* game data setup */
    client.player = (player_t){
        .cameraHeight = 1.5,
        .radius = 0.25,
        .height = 1.9,
        .speed = 6,
        .gravity = 12,
        .jump=7,
        .move = NORMAL,
        .world = 0,
        .client = &client,
    };
    thing_t blockHighlight = {
        .prog = client.shaders[0],
        .tex = 0,
        .vao = createCubeMesh(),
        .n = 36
    };
    mat4Identity(blockHighlight.transform);
    thing_t crosshair = {
        .prog = client.shaders[2],
        .tex = 0,
        .vao = createCubeMesh(),
        .n = 6
    };
    mat4Scale(crosshair.transform, 0.01, 0.01, 0.01);
    double currentTime = 0;
    double begin = glfwGetTime();
    double lastTime = 0;
    int paused = 0;

    typedef enum {GAMESTATE_MAINMENU, GAMESTATE_LOADING, GAMESTATE_INGAME} gamestate;

    gamestate GameState = GAMESTATE_MAINMENU;
    
    /* main loop */
    while (!glfwWindowShouldClose(window)) {
        /* stuff that should happen on every frame */

        // TODO : fix . bad!
        currentTime = glfwGetTime() - begin;
        double delta = currentTime - lastTime;
        lastTime = currentTime;
        
        gInput.mouseDeltaX = 0.0;
        gInput.mouseDeltaY = 0.0;
        gInput.scrollY = 0.0;
        glfwGetFramebufferSize(window, &fb_width, &fb_height);
        glViewport(0, 0, fb_width, fb_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);
        
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
        
        /* game states */
        
        switch (GameState) {
            case GAMESTATE_MAINMENU: {
                static int attemptingConnect = 0;
                static int showConnectMenu = 0;
                static int showLoadMenu = 0;
                static int showPreferencesMenu = 0;
                static int showCreateWorldMenu = 0;
                if (button_once(&ctx, 60, 200, "create world", "generate a new world")) {
                    showCreateWorldMenu = !showCreateWorldMenu;
                    showConnectMenu = 0;
                    showPreferencesMenu = 0;
                }
                button_once(&ctx, 60, 300, "load world", "load world from file");
                if (button_once(&ctx, 60, 400, "join world", "join an online world")) {
                    showCreateWorldMenu = 0;
                    showLoadMenu = 0;
                    showConnectMenu = !showConnectMenu;
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
            
                    if ( button_once(&ctx, 650, 600, "create", "create the world!")) {
                        pthread_t createWorldThread;
                        client.world = (struct world){
                            .shaderProgram = client.shaders[1],
                            .block_tex_lut = calloc(1, 6 * sizeof(unsigned)),
                            .size_block_tex_lut = 1
                        };
                        client.player.world = &client.world;
                        void * createWorld(void * arg); 
                        pthread_create(&createWorldThread, NULL, createWorld, &client.world);
                        pthread_detach(createWorldThread);
                        showCreateWorldMenu = 0;
                        GameState = GAMESTATE_LOADING;
                    }
                }
                if (showConnectMenu) {
                    static textfield_t ipAddrTextfield = {0};
                    textfield(&ctx, &ipAddrTextfield, 650, 400, "ip", 10, "ok");
                    static textfield_t portTextfield = {0};
                    textfield(&ctx, &portTextfield, 650, 500, "port", 10, "ok");
                    if (button_once(&ctx, 650, 600, "connect", "ok")) {
                        attemptingConnect = 1;
                    }
                }
                if (attemptingConnect) {
                    if (!showConnectMenu) {
                        // destroy connect thread
                        attemptingConnect = 0;
                    }
                    drawText("attempting connect", 650, 700, fb_width,fb_height,8,16);
                    if (client.isConnectedToServer) {
                    }
                }
                break;
            }
            case GAMESTATE_LOADING: {
                char loadingMessage[30];
                sprintf(loadingMessage, "Creating world %d %%", client.world.creationProgress);
                drawText(loadingMessage, 650, 600, fb_width,fb_height,8,16);
                if (client.world.creationProgress == 512) {
                    GameState = GAMESTATE_INGAME;}
                break;
            }
            case GAMESTATE_INGAME: {
                glClearColor(0.25, 0.25, 0.5, 0);
                if (gInput.keys[GLFW_KEY_ESCAPE]) { 
                    paused = !paused;
                    glfwSetInputMode(window, GLFW_CURSOR, paused ? GLFW_CURSOR_NORMAL: GLFW_CURSOR_DISABLED);
                    gInput.keys[GLFW_KEY_ESCAPE] = 0;
                }
                /* movement */
                if (!paused) processPlayerMovement(&client.player, delta);
                /* calculate view matrix */
                mat4 m1;            mat4RotateX(m1,client.player.pitch);
                mat4 m2;            mat4RotateY(m2,client.player.yaw);
                mat4 m3;            mat4Multiply (m3, m1, m2);
                mat4 m4;            mat4Translate(m4, -client.player.pos.x, -(client.player.pos.y+client.player.cameraHeight+0.01*sin(currentTime*7)), -client.player.pos.z);
                mat4 viewMatrix;    mat4Multiply (viewMatrix,m3, m4);
                glUseProgram(client.shaders[1]); glUniformMatrix4fv(glGetUniformLocation(client.shaders[1],"view"), 1, GL_FALSE, viewMatrix);
                glUseProgram(client.shaders[0]); glUniformMatrix4fv(glGetUniformLocation(client.shaders[0],"view"), 1, GL_FALSE, viewMatrix);

                static int wireframe = 0;

                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
                glUseProgram(client.shaders[1]);

                glEnable(GL_DEPTH_TEST);
                void drawWorld(struct world * w, fvec3 pos, int dh, int dv);
                static unsigned color = 0xFFFFFFFF;
                float a = ((color >> 24) & 0xFF) / 255.0;
                float r = ((color >> 16) & 0xFF) / 255.0;
                float g = ((color >> 8)  & 0xFF) / 255.0;
                float b = (color & 0xFFu) / 255.0;
                //assumes that viewmodel has been set
                drawWorld(&client.world, client.player.pos, 5, 5);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);
                
                ivec3 hit;
                int hitfound = voxelRaycastHit(client.world,(fvec3){ client.player.pos.x , client.player.pos.y + client.player.cameraHeight, client.player.pos.z}, client.player.yaw - M_PI * 0.5, client.player.pitch * -1, 5, &hit);
                if (hitfound) {
                    mat4Translate(blockHighlight.transform, hit.x+0.5, hit.y+0.5, hit.z+0.5);
                } else {
                    mat4Scale(blockHighlight.transform, 0, 0, 0);
                }
                drawThing(&blockHighlight);
                
                glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);
                slider(&ctx, 32,300, &client.player.speed, 0.0, 200.0, "speed");
                slider(&ctx, 32,400, &client.player.gravity, 0.0, 20.0, "gravity");
                slider(&ctx, 32,500, &client.player.jump, 0.0, 20.0, "jump");
                static int flycheckbox = 1;
                checkbox(&ctx,32,600, &flycheckbox, "flycheckbox");
                client.player.move = flycheckbox ? FLY : NORMAL;
                checkbox(&ctx,32,700, &wireframe, "wireframe");
                colorpicker(&ctx, 400, 800, &color, "test");
                
                static GLuint atlasTexture = 0;
                static unsigned atlasTexIndex = 0;
                static unsigned * dataPtr = 0;
                static unsigned nLayers = 3;
                atlasedit(&ctx, 32, 800, 16, 16, &nLayers, atlasTexIndex, color, &dataPtr, &atlasTexture );
                client.world.tex = atlasTexture;
            
                if (button_once(&ctx, 32, 1080, "+","increment tex")) {atlasTexIndex ++;};  
                if (button_once(&ctx, 128, 1080, "-","decrement tex")) atlasTexIndex--;
                if (button_once(&ctx, 256, 1080, "sav","save texture atlas")) {
                    FILE * fptr = fopen("atlas.texatlas", "wb");
                    fputc(nLayers, fptr);
                    fwrite(dataPtr, sizeof(unsigned), 16 * 16 * nLayers, fptr);
                    fclose(fptr);
                    fptr = fopen("blocktypes", "wb");
                    fputc(client.world.size_block_tex_lut, fptr);
                    fwrite(client.world.block_tex_lut, sizeof(unsigned), client.world.size_block_tex_lut * 6, fptr);
                    fclose(fptr);
                };
                static GLuint blocktypeLUT = 0;
                if (!blocktypeLUT) {
                    glGenTextures(1, &blocktypeLUT);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_1D, blocktypeLUT);
                    glTexImage1D(
                        GL_TEXTURE_1D,
                        0,
                        GL_R32I,           // each texel = 32-bit signed integer
                        6 * client.world.size_block_tex_lut,            // layers (y dimension)
                        0,
                        GL_RED_INTEGER,    // must be *_INTEGER for integer formats
                        GL_INT,            // type of your data
                        client.world.block_tex_lut
                    );
                    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glActiveTexture(GL_TEXTURE0);
                }
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
                    fptr = fopen("blocktypes", "rb");
                    if (fptr) {
                        fread(&client.world.size_block_tex_lut, sizeof(unsigned char), 1, fptr);
                        client.world.block_tex_lut = realloc(client.world.block_tex_lut, client.world.size_block_tex_lut * 6 * sizeof(unsigned));
                        fread(client.world.block_tex_lut, sizeof(unsigned), client.world.size_block_tex_lut * 6, fptr);
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_1D, blocktypeLUT);
                        glTexImage1D(
                            GL_TEXTURE_1D,
                            0,
                            GL_R32I,           // each texel = 32-bit signed integer
                            6 * client.world.size_block_tex_lut,            // layers (y dimension)
                            0,
                            GL_RED_INTEGER,    // must be *_INTEGER for integer formats
                            GL_INT,            // type of your data
                            client.world.block_tex_lut
                        );
                        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glActiveTexture(GL_TEXTURE0);
                    }
                };
                {
                    char buffer[5];
                    sprintf(buffer, "%d",client.blockType);
                    drawText(buffer, 32, 1200, fb_width, fb_height, 8, 16);
                }
            
                
            
                if (button_once(&ctx, 32, 1380, "+","increment block type")) {
                    client.blockType ++;
                    if (client.blockType == client.world.size_block_tex_lut) {
                        client.world.size_block_tex_lut ++;
                        client.world.block_tex_lut = realloc(client.world.block_tex_lut, client.world.size_block_tex_lut * 6 * sizeof(unsigned));
                        //memset(client.world.block_tex_lut[6*client.world.size_block_tex_lut - 6], 0, 6 * sizeof(unsigned));
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_1D, blocktypeLUT);
                        glTexImage1D(
                            GL_TEXTURE_1D,
                            0,
                            GL_R32I,           // each texel = 32-bit signed integer
                            6 * client.world.size_block_tex_lut,            // layers (y dimension)
                            0,
                            GL_RED_INTEGER,    // must be *_INTEGER for integer formats
                            GL_INT,            // type of your data
                            client.world.block_tex_lut
                        );
                        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glActiveTexture(GL_TEXTURE0);
                    }
                };
                if (button_once(&ctx, 128, 1380, "-","decrement block type")) {
                    client.blockType--;
                    if (client.blockType < 0) client.blockType = 0;
                }
                
                // Bank of 6 faces
                // Bank of 6 faces (row per face)
                const int xSpacing = 100;  // horizontal spacing between buttons and label
                const int ySpacing = 100;   // vertical spacing between rows
                const int startX = 400;     // starting x position
                const int startY = 1200;   // starting y position

                for (int face = 0; face < 6; face++) {
                    int rowY = startY + face * ySpacing;

                    unsigned texIndex = client.world.block_tex_lut[client.blockType * 6 + face];
                    
                    if (button_once(&ctx, startX, rowY, "-", "decrement texture id")) {
                        if (client.world.block_tex_lut[client.blockType * 6 + face] > 0)
                            client.world.block_tex_lut[client.blockType * 6 + face]--;
                        
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_1D, blocktypeLUT);
                        glTexSubImage1D(
                            GL_TEXTURE_1D,
                            0,
                            face + 6 * client.blockType,     
                            1,                    
                            GL_RED_INTEGER,
                            GL_INT,
                            &client.world.block_tex_lut[client.blockType * 6 + face]
                        );
                        glActiveTexture(GL_TEXTURE0);
                        
                    }

                    if (button_once(&ctx, startX + xSpacing, rowY, "+", "increment texture id")) {
                        client.world.block_tex_lut[client.blockType * 6 + face]++;
                        
                        glActiveTexture(GL_TEXTURE1);
                        glBindTexture(GL_TEXTURE_1D, blocktypeLUT);
                        glTexSubImage1D(
                            GL_TEXTURE_1D,
                            0,
                            face + 6 * client.blockType,
                            1,
                            GL_RED_INTEGER,
                            GL_INT,
                            &client.world.block_tex_lut[client.blockType * 6 + face]
                        );
                        glActiveTexture(GL_TEXTURE0);
                        
                    }

                    // Draw label after the buttons
                    char label[32];
                    sprintf(label, "face %d: %d", face, texIndex);
                    drawText(label, startX + 2 * xSpacing + 16, rowY, fb_width, fb_height, 8, 16);
                }
                
                char buff[80]; sprintf(buff, "xyz %5d %5d %5d fps", (int)client.player.pos.x, (int)client.player.pos.y, (int)client.player.pos.z);
                drawText(buff,32,0, fb_width, fb_height, 8, 16);
                char buff2[30];
                sprintf(buff2, "vel %5d %5d %5d", (int)client.player.vel.x, (int)client.player.vel.y, (int)client.player.vel.z);
                drawText(buff2,32,128, fb_width, fb_height, 8, 16);
                glEnable(GL_DEPTH_TEST);
                drawThing(&crosshair);
                break;
            }
        }
        // END FRAME
        gInput.typedChar = 0;
        glfwSwapBuffers(window);
    }
    
    /* cleanup */
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}