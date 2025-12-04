#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pthread.h>


#include "main.h"

#include <dirent.h>
#include <tgmath.h>

#include "gui.h"
#include "math.h"
#include "voxels.h"
#include <sys/stat.h>
#include <math.h>

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
    inputInit(window); // TODO : this shouldnt be a function
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    /* graphics pipeline setup */
    struct client client;
    client.shaders[0] = shaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
    client.shaders[1] = shaderProgram("shaders/world.vert", "shaders/world.frag");
    client.shaders[2] = shaderProgram("shaders/crosshair.vert", "shaders/crosshair.frag");
    client.shaders[3] = shaderProgram("shaders/sun.vert", "shaders/sun.frag");

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
    // sun shader
    glUseProgram(client.shaders[3]);
    glUniform1i(glGetUniformLocation(client.shaders[3], "tex0"), 0);
    //glUniform1f(glGetUniformLocation(client.shaders[3], "size"), 20);
    //glUniform1i(glGetUniformLocation(client.shaders[3], "radius"), 10);
    
    glUniformMatrix4fv(glGetUniformLocation(client.shaders[3],"projection"), 1, GL_FALSE, proj);
    
    /* game data setup */
    const struct player PLAYER_DEFAULTS = (player_t){
        .cameraHeight = 1.5,
        .radius = 0.25,
        .height = 1.9,
        .speed = 6,
        .longlegs = 0,
        .gravity = 12,
        .jump=7,
        .move = NORMAL,
        .world = NULL,
        .client = &client,
    };

    client.meshing_queue = (struct chunk_queue){0};
    
    client.player = PLAYER_DEFAULTS;
    // todo hmm....
    client.world = (struct world){
        .shaderProgram = client.shaders[1],
        .block_tex_lut = calloc(1, 6 * sizeof(unsigned)),
        .size_block_tex_lut = 1,
        .size_h = 16,
        .size_v = 8,
        .name = "world"
    };
    
    client.player.world = &client.world;
    
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
    thing_t sun = {
        .prog = client.shaders[3],
        .tex = 0,
        .vao = 0,
        .n = 6
    };
    
    {
        float sunVertices[8] = {-0.5f,-0.5f,0.5f,-0.5f,-0.5f,0.5f,0.5f,0.5f};
        unsigned sunIndices[6] = {0,1,2,1,2,3};
        opengl_mesh sunMesh = create_opengl_mesh(sunVertices, sizeof(sunVertices), sunIndices, 6, 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
        sun.vao = sunMesh.vao;
    }
    
    double currentTime = 0;
    double begin = glfwGetTime();
    double lastTime = 0;
    int paused = 1;

    typedef enum {GAMESTATE_MAINMENU, GAMESTATE_GENERATING, GAMESTATE_INGAME} gamestate;

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
        glEnable(GL_DEPTH_TEST);
        //glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);
        
        struct guictx ctx = {
            .screenWidth = fb_width,
            .screenHeight = fb_height,
            .mx = paused ? gInput.mouseX * 2 : 0 ,
            .my = paused ? gInput.mouseY * 2 : 0,
            .mb = paused ? gInput.mouseButtons[GLFW_MOUSE_BUTTON_LEFT] : 0,
            .typedChar =  gInput.typedChar,
            .textScaleX = 8,
            .textScaleY = 16,
            .right = fb_width / 0.75,
            .left = 0,
            .layout = GUI_LAYOUT_MANUAL,
            .spacing = 32,
            .keys = gInput.keys,
            .gui_scale = 0.75};
        
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
                    showLoadMenu = 0;
                }
                if (button_once(&ctx, 60, 300, "load world", "load world from file")) {
                    
                    showLoadMenu = !showLoadMenu;
                    showCreateWorldMenu = 0;
                    showConnectMenu = 0;
                    showPreferencesMenu = 0;
                    
                }
                if (button_once(&ctx, 60, 400, "join world", "join an online world")) {
                    showCreateWorldMenu = 0;
                    showLoadMenu = 0;
                    showPreferencesMenu = 0;
                    showConnectMenu = !showConnectMenu;
                }
                static int askForOverwrite = 0;
                if (showCreateWorldMenu) {
                    static textfield_t worldNameTextfield = {0};
                    textfield(&ctx, &worldNameTextfield, 650, 200, "world name", 15, "test");
                    if ( button_once(&ctx, 650, 300, "create!", "create the world!")) {
                      	// see if subdirectory exists
                        client.world.name = worldNameTextfield.buffer;
                        struct stat st = {0};
   		 				if (stat(client.world.name, &st) != 0) {
                            pthread_t createWorldThread;
                        	void * createWorld(void * arg); 
                        	pthread_create(&createWorldThread, NULL, createWorld, &client.world);
                        	pthread_detach(createWorldThread);
                        	showCreateWorldMenu = 0;
                        	GameState = GAMESTATE_GENERATING;                       
    					} else {
                        	askForOverwrite = 1;                 
    					}
                    }
                    
                    if (askForOverwrite) {
                    	textlabel(&ctx, "dir exists. overwrite?", 650, 400, NULL);
                        if (button_once(&ctx, 650, 500, "yes, overwrite!", "overwrite the data")) {
                        	//DIR *dir = opendir(client.world.name);
                            // delete everything in the folder recursively   
                            pthread_t createWorldThread;
                        	void * createWorld(void * arg); 
                        	pthread_create(&createWorldThread, NULL, createWorld, &client.world);
                        	pthread_detach(createWorldThread);
                        	showCreateWorldMenu = 0;
                        	GameState = GAMESTATE_GENERATING;     
                        }
                        
                    }
                }
                
                if (showLoadMenu) {
                    
                    DIR *dir = opendir(".");
                    if (!dir) exit(1);
                    struct dirent * ent;
                    int count = 0;
                    while ((ent = readdir(dir))) {
                        if (ent->d_type != DT_DIR) continue;
                        if (ent->d_name[0] == '.') continue;
                        char worldInfoPath[128];
                        sprintf(worldInfoPath, "%s/worldinfo", ent->d_name);
                        struct stat st = {0};
                        if (stat(worldInfoPath, &st) != 0) continue;
                        static char worldNameBuffer[128];
                        
                        if (button_once(&ctx, 650, 100 + 100 * count, ent->d_name, "load this world")) {
                            strcpy(worldNameBuffer, ent->d_name);
                            client.world.name = worldNameBuffer;
                            loadWorld(&client.world);
                            char fnBuff[128];
                            sprintf(fnBuff, "%s/player.txt", worldNameBuffer);
                            FILE * fptr = fopen(fnBuff,"r");
                            if (fptr)
                                fscanf(fptr,"player pos %f %f %f vel %f %f %f pitch/yaw(radians) %f %f",
                                    &client.player.pos.x,
                                    &client.player.pos.y,
                                    &client.player.pos.z,
                                    &client.player.vel.x,
                                    &client.player.vel.y,
                                    &client.player.vel.z,
                                    &client.player.pitch,
                                    &client.player.yaw);
                            fclose(fptr);
                            
                            GameState = GAMESTATE_INGAME;
                            showLoadMenu = 0;
                        }
                        count++;
                    }
                    closedir(dir);
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
                    textlabel(&ctx, "attempting connect", 650, 700, "");
                    if (client.isConnectedToServer) {
                    }
                }
                break;
            }
            case GAMESTATE_GENERATING: {
                printf("generating world...\n");
                char loadingMessage[30];
                sprintf(loadingMessage, "Creating world %d %%", client.world.creationProgress);
                textlabel(&ctx, loadingMessage, 650, 600, "");
                if (client.world.creationProgress == -1) {
                    saveWorld(&client.world);
                    printf("generating world 1...\n");
                    GameState = GAMESTATE_INGAME;}
                break;
            }
            case GAMESTATE_INGAME: {
                
                static double sunDirX, sunDirY, sunDirZ = 0;
                
                /* pause / unpause */
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
                glUseProgram(client.shaders[3]); glUniformMatrix4fv(glGetUniformLocation(client.shaders[3],"view"), 1, GL_FALSE, viewMatrix);
                glUniform3f(glGetUniformLocation(client.shaders[3], "sunDir"), sunDirX, sunDirY, sunDirZ);

                glUseProgram(client.shaders[0]); glUniformMatrix4fv(glGetUniformLocation(client.shaders[0],"view"), 1, GL_FALSE, viewMatrix);

                glUseProgram(client.shaders[1]); glUniformMatrix4fv(glGetUniformLocation(client.shaders[1],"view"), 1, GL_FALSE, viewMatrix);
                glUniform1f(glGetUniformLocation(client.shaders[1], "time"), currentTime);

                glUniform3f(glGetUniformLocation(client.shaders[1], "sunDir"), sunDirX, sunDirY, sunDirZ);
                
                glUniform3f(glGetUniformLocation(client.shaders[1], "cameraPos"), client.player.pos.x, client.player.pos.y, client.player.pos.z);
                static int wireframe = 0;
                
                /* draw the sun */

                if (gInput.keys[GLFW_KEY_X]) {
                    sunDirX = (cos(client.player.pitch) * sin(client.player.yaw));
                    sunDirY = -(sin(client.player.pitch));
                    sunDirZ = -(cos(client.player.pitch) * cos(client.player.yaw));
                }
                
                glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
                glEnable(GL_DEPTH_TEST);
                glUseProgram(sun.prog);
                glBindVertexArray(sun.vao);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, sun.tex);
                glUniform1i(glGetUniformLocation(sun.prog, "tex0"), 0);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
                glBindVertexArray(0);
                
                /* draw the world */
                glEnable(GL_CULL_FACE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                void drawAllChunks(struct world * w, struct chunk_queue * meshing_queue);
                drawAllChunks(&client.world, &client.meshing_queue);
                
                /* looking-at-block outline */
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

                /* in-game gui */
                glEnable(GL_DEPTH_TEST);
                glPolygonMode(GL_FRONT_AND_BACK,  GL_FILL);
                ctx.layout = GUI_LAYOUT_AUTO;
                char buff[80]; sprintf(buff, "xyz %5d %5d %5d", (int)client.player.pos.x, (int)client.player.pos.y, (int)client.player.pos.z);
                textlabel(&ctx, buff,32,0, "coords");
                char buff2[30];
                sprintf(buff2, "vel %5d %5d %5d", (int)client.player.vel.x, (int)client.player.vel.y, (int)client.player.vel.z);
                textlabel(&ctx, buff2, 32,128, "vel");
                if (paused) {
                    static double fogStart = 0, fogEnd = 2048;
                    slider(&ctx, 0, 0, &fogStart, 0.0, 1024.0, "fog start");
                    slider(&ctx, 0, 0, &fogEnd, 0.0, 2048.0, "fog end");
                    glUseProgram(client.shaders[1]);
                    glUniform1f(glGetUniformLocation(client.shaders[1], "fogStart"), fogStart);
                    glUniform1f(glGetUniformLocation(client.shaders[1], "fogEnd"), fogEnd);

                    slider(&ctx, 32,300, &client.player.height, 0.0, 64.0, "height");
                    client.player.cameraHeight = 0.80 * client.player.height;
                    slider(&ctx, 32,400, &client.player.radius, 0.0, 16.0, "radius");
                    
                    slider(&ctx, 32,300, &client.player.speed, 0.0, 200.0, "speed");
                    slider(&ctx, 32,400, &client.player.gravity, 0.0, 200.0, "gravity");
                    static double longlegs = 0;
                    slider(&ctx, 32,400, &longlegs, 0.0, 4.0, "longlegs");
                    client.player.longlegs = longlegs;
                    slider(&ctx, 32,500, &client.player.jump, 0.0, 200.0, "jump");
                    static unsigned lightColor = 0xFFFFFFFF;
                    colorpicker(&ctx, 300, 300, &lightColor, "light color");
                    static unsigned bgColor = 0xFFFFFFFF;
                    colorpicker(&ctx, 300, 300, &bgColor, "sky color");
                    static unsigned fogColor = 0xFFFFFFFF;
                    colorpicker(&ctx, 300, 300, &fogColor, "fog color");
                    if (button_once(&ctx, 600, 300, "save", "save the world")) {
                        saveWorld(&client.world);
                        char fnBuff[128];
                        sprintf(fnBuff, "%s/player.txt", client.world.name);
                        FILE * fptr = fopen(fnBuff,"w");
                        if (!fptr) exit(102);
                        fprintf(fptr,"player pos %f %f %f vel %f %f %f pitch/yaw(radians) %f %f",client.player.pos.x,client.player.pos.y,client.player.pos.z,client.player.vel.x,client.player.vel.y,client.player.vel.z,client.player.pitch,client.player.yaw);
                        fclose(fptr);
                    }
                    if (button_once(&ctx, 600, 300, "quit", "save the world")) {
                        freeWorld(&client.world);
                        GameState = GAMESTATE_MAINMENU;
                    }
                    
                    fcolor_rgba lightColorVector = colorFromUint(lightColor);
                    fcolor_rgba fogColorVector = colorFromUint(fogColor);
                    fcolor_rgba skyColorVector = colorFromUint(bgColor);
                    
                    
                    glUseProgram(client.shaders[1]);


                    
                    glUniform3f(glGetUniformLocation(client.shaders[1], "lightColor"), lightColorVector.r, lightColorVector.g, lightColorVector.b);
                    glUniform3f(glGetUniformLocation(client.shaders[1], "fogColor"), fogColorVector.r, fogColorVector.g, fogColorVector.b);
                    glClearColor(skyColorVector.r, skyColorVector.g, skyColorVector.b, 0);

                    static int flycheckbox = 1;
                    checkbox(&ctx,32,600, &flycheckbox, "flycheckbox");
                    client.player.move = flycheckbox ? FLY : NORMAL;
                    checkbox(&ctx,32,700, &wireframe, "wireframe");

                    /* texture editor / block type editor gui */
                    
                    static unsigned paint_color = 0xFFFFFFFF;
                    colorpicker(&ctx, 400, 800, &paint_color, "test");
                    static GLuint atlasTexture = 0;
                    sun.tex = atlasTexture;
                    static unsigned atlasTexIndex = 0;
                    static unsigned * dataPtr = 0;
                    static unsigned nLayers = 3;
                    char texIndexBuffer[16];
                    sprintf(texIndexBuffer, "tex %3d", atlasTexIndex);
                    textlabel(&ctx, texIndexBuffer, 0, 0, "texture id");
                    atlasedit(&ctx, 32, 800, 16, 16, &nLayers, atlasTexIndex, paint_color, &dataPtr, &atlasTexture );
                    client.world.tex = atlasTexture;
                    if (button_once(&ctx, 32, 1080, "+","increment tex")) {atlasTexIndex ++;};  
                    if (button_once(&ctx, 128, 1080, "-","decrement tex")) atlasTexIndex--;
                    if (button_once(&ctx, 256, 1080, "sav","save texture atlas")) {
                        FILE * fptr = fopen("atlas", "wb");
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
                    FILE * fptr = fopen("atlas", "rb");
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
                        char buffer[32];
                        sprintf(buffer, "block type %3d",client.blockType);
                        textlabel(&ctx, buffer, 0, 0, "selected block type");
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
                    kxguiNewline(&ctx);
                    // legacy
                    const int xSpacing = 100;  
                    const int ySpacing = 100;   
                    const int startX = 400;     
                    const int startY = 1200;  
                    for (int face = 0; face < 6; face++) {
                        int rowY = startY + face * ySpacing;

                        unsigned texIndex = client.world.block_tex_lut[client.blockType * 6 + face];

                        // Draw label after the buttons
                        char label[32];
                        sprintf(label, "face %d: %3d", face, texIndex);
                        textlabel(&ctx, label, startX + 2 * xSpacing + 16, rowY, "");
                    
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
                        glTexSubImage1D(GL_TEXTURE_1D,0,face + 6 * client.blockType,1,GL_RED_INTEGER,GL_INT,&client.world.block_tex_lut[client.blockType * 6 + face]);
                            glActiveTexture(GL_TEXTURE0);
                        }
                        kxguiNewline(&ctx);
                    }
                    if (button_once(&ctx, 0, 0, "recompile shaders!", "yep!")) {
                        /* graphics pipeline setup */
                        client.shaders[0] = shaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
                        client.shaders[1] = shaderProgram("shaders/world.vert", "shaders/world.frag");
                        client.shaders[2] = shaderProgram("shaders/crosshair.vert", "shaders/crosshair.frag");
                        client.shaders[3] = shaderProgram("shaders/sun.vert", "shaders/sun.frag");

                        //mat4 proj; mat4Proj(proj, 1.5707963f, (float)WIDTH / (float)HEIGHT, 0.1f, 2048.0f);
                        // default shader
                        glUseProgram(client.shaders[0]);
                        glUniform1i(glGetUniformLocation(client.shaders[0], "tex"), 0);
                        glUniformMatrix4fv(glGetUniformLocation(client.shaders[0],"projection"), 1, GL_FALSE, proj);
                        // world shader
                        glUseProgram(client.shaders[1]);
                        glUniform1i(glGetUniformLocation(client.shaders[1], "tex0"), 0);
                        glUniform1i(glGetUniformLocation(client.shaders[1], "tex1"), 1);
                        glUniformMatrix4fv(glGetUniformLocation(client.shaders[1],"projection"), 1, GL_FALSE, proj);
                        // sun shader
                        glUseProgram(client.shaders[3]);
                        glUniform1i(glGetUniformLocation(client.shaders[3], "tex0"), 0);
                        //glUniform1f(glGetUniformLocation(client.shaders[3], "size"), 20);
                        //glUniform1i(glGetUniformLocation(client.shaders[3], "radius"), 10);
    
                        glUniformMatrix4fv(glGetUniformLocation(client.shaders[3],"projection"), 1, GL_FALSE, proj);
                        client.world.shaderProgram = client.shaders[1];
                        blockHighlight.prog = client.shaders[0];
                        crosshair.prog = client.shaders[2];
                        sun.prog = client.shaders[3];
                    }
                }
                glEnable(GL_DEPTH_TEST);
                drawThing(&crosshair);
                
                /* run the mesher on one chunk */
                struct chunk * chunkToMesh = dequeueChunk(&client.meshing_queue);
                if (!chunkToMesh) break;
                chunkToMesh->needsRemesh = false;
                voxelMeshData(chunkToMesh, &client.world);
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