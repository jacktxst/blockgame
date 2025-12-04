//
// Created by jack lewis on 11/15/25.
//

#ifndef TEXT_H
#define TEXT_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KXGUI_CHAR_W 6
#define KXGUI_CHAR_H 6
#define KXGUI_FONT_COLS 8
#define KXGUI_FONT_ROWS 12

#define KXGUI_DEBUG
static float KXGUI_UNIT_QUAD_VERTS[8] = {
  0.0f, 0.0f,
  1.0f, 0.0f,
  0.0f, 1.0f,
  1.0f, 1.0f
};
static const char * KXGUI_FONT =
"         #    # #   # #  ##### ##  #   #     #  \n"
"         #    # #  ##### # #   ## #   # #    #  \n"
"         #          # #  #####   #    ## #      \n"
"                   #####   # #  # ## #  #       \n"
"         #          # #  ##### #  ##  ## #      \n"
"                           #                    \n"
"  ##     ##  # # #   #                         #\n"
" #         #  ###    #                        # \n"
" #         # ##### #####       #####         #  \n"
" #         #  ###    #     #                #   \n"
"  ##     ##  # # #   #    ##           #   #    \n"
"                                                \n"
" #####   #    ###  ##### #  #  ##### ##### #####\n"
" #  ##  ##   #   #     # #  #  #     #         #\n"
" # # # # #     ##   #### ##### ####  #####    # \n"
" ##  #   #    #        #    #      # #   #    # \n"
" ##### ##### ##### #####    #  ####  #####   #  \n"
"                                                \n"
" ##### #####                 #       #     #####\n"
" #   # #   #   #     #     ##  #####  ##   #   #\n"
" ##### #####             ##             ##    # \n"
" #   #     #   #     #     ##  #####  ##        \n"
" ##### #####        ##       #       #       #  \n"
"                                                \n"
" ##### ##### ####  ##### ####  ##### ##### #####\n"
" #   # #   # #   # #     #   # #     #     #    \n"
" # ### ##### ####  #     #   # ##### ##### #  ##\n"
" # # # #   # #   # #     #   # #     #     #   #\n"
" # ### #   # ##### ##### ####  ##### #     #####\n"
"                                                \n"
" #   # ##### ##### #   # #     ##### #   # #####\n"
" #   #   #     #   #  #  #     # # # ##  # #   #\n"
" #####   #     #   ###   #     # # # # # # #   #\n"
" #   #   #     #   #  #  #     # # # #  ## #   #\n"
" #   # ##### ###   #   # ##### #   # #   # #####\n"
"                                                \n"
" ##### ##### ##### ##### ##### #   # #   # # # #\n"
" #   # #   # #   # #       #   #   # #   # # # #\n"
" ##### # # # ##### #####   #   #   # #   # # # #\n"
" #     #  ## #  #      #   #   #   #  # #  # # #\n"
" #     ##### #   # #####   #   #####   #   #####\n"
"                                                \n"
" #   # #   # ##### ###    #      ###   #        \n"
"  # #   # #     #  #      #        #  # #       \n"
"   #     #     #   #       #       # #   #      \n"
"  # #    #    #    #       #       #            \n"
" #   #   #   ##### ###      #    ###       #####\n"
"                            #                   \n"
" #     ####  #               #  ###   ###   ####\n"
"  #        # #      ####  #### #   # #   # #   #\n"
"       ##### ####  #     #   # ####  ###    ####\n"
"       #   # #   # #     #   # #     #         #\n"
"       ##### ####   ####  ####  ###  #      ### \n"
"                                                \n"
" #                 #                            \n"
" #       #     #   #     #     #     #          \n"
" ####              # #   #     ##### ####  #### \n"
" #   #   #     #   ##    #     # # # #  #  #  # \n"
" #   #   #   # #   # #   #     # # # #  #  #### \n"
"             ###                                \n"
"                    #                           \n"
"             #     #       #                    \n"
" ###    ###  ###    #     ###  #  #   # #  # # #\n"
" #  #  #  #  #  #    #     #   #  #   # #  # # #\n"
" ###    ###  #      #      #    ##     #    # # \n"
" #        #                                     \n"
"                                                \n"
"              ####   ##    #    ##              \n"
"  # #   # #     #   #      #      #   # #       \n"
"   #    ###    #   ##      #      ## # #        \n"
"  # #     #   ####  #      #      #             \n"
"         #           ##    #    ##              \n";
static const char * glyphVertShaderSrc =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;         \n"
"uniform ivec2 screen_size;                 \n"
"uniform ivec2 origin;                      \n"
"uniform int   z;\n"
"uniform ivec2 scale;                       \n"
"\n"
"uniform ivec2 font_size;                   \n"
"uniform int   char_index;                  \n"
"uniform ivec2 char_size;                   \n"
"\n"
"out     vec2  uv;\n"
"void main() {\n"
"  gl_Position = vec4((vec2(aPos*char_size*scale+origin)/vec2(screen_size)*2.0-1.0)*vec2(1,-1),float(z)/16.0,1.0);\n"
"\n"
"  int col = char_index % font_size.x;\n"
"  int row = char_index / font_size.x;\n"
"  vec2 glyphSize = vec2(1.0 / float(font_size.x), 1.0 / float(font_size.y));\n"
"  vec2 glyphUV = vec2(float(col), float(row)) * glyphSize;\n"
"  uv = aPos * glyphSize + vec2(col, row) * glyphSize;\n"
" }\n";
static const char * glyphFragShaderSrc =
"#version 330 core\n"
"in vec2 uv;\n"
"out vec4 fragColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"    float a = texture(tex, uv).r;\n"
"    fragColor = vec4(uv, 1.0, a);\n" // using the uv for the color here creates a unique stylistic effect (bad for readability)
"}\n";
static const char * rectVertShaderSrc =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;         \n"
"uniform ivec2 screen_size;                 \n"
"uniform ivec2 origin;                      \n"
"uniform ivec2 scale;                       \n"
"uniform int   z;"
"out     vec2  uv;\n"
"void main() {\n"
"  gl_Position = vec4((vec2(aPos*scale+origin)/vec2(screen_size)*2.0-1.0)*vec2(1,-1),float(z)/16.0,1.0);\n"
"  uv = aPos;\n"
" }\n";
static const char * atlasRectFragShaderSrc =
"#version 330 core\n"
"in vec2 uv;\n"
"out vec4 fragColor;\n"
"uniform sampler2DArray tex;\n"
"uniform int textureIndex;\n"
"void main(){\n"
"    fragColor = vec4(texture(tex, vec3(uv, textureIndex)).rgb, 1.0);\n"
"}\n";
static const char * colorRectFragShaderSrc =
"#version 330 core\n"
"out vec4 fragColor;\n"
"uniform vec4 color;\n"
"void main(){\n"
"    fragColor = color;\n"
"}\n";
static void drawColorRect(int x, int y, int z, int w, int h, int screenW, int screenH,
  float red, float green, float blue, float alpha) {
  static GLuint
  colorRectShaderProg = 0,
  vao = 0,
  vbo = 0,
  screenSizeLoc = 0,
  zLoc = 0,
  scaleLoc = 0,
  colorLoc = 0,
  originLoc = 0;
  if (!colorRectShaderProg) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &rectVertShaderSrc, NULL);
    glCompileShader(vertexShader);
    GLint ok = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(vertexShader, sizeof(log), NULL, log);
      fprintf(stderr, "vertex shader compile error: %s\n", log);
      exit(1);
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &colorRectFragShaderSrc, NULL);
    glCompileShader(fragmentShader);
    ok = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(fragmentShader, sizeof(log), NULL, log);
      fprintf(stderr, "frag shader compile error: %s\n", log);
      exit(1);
    }
    colorRectShaderProg = glCreateProgram();
    glAttachShader(colorRectShaderProg, vertexShader);
    glAttachShader(colorRectShaderProg, fragmentShader);
    glLinkProgram(colorRectShaderProg);
    ok = 0;
    glGetProgramiv(colorRectShaderProg, GL_LINK_STATUS, &ok);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!ok) {
      char log[512];
      glGetProgramInfoLog(colorRectShaderProg, sizeof(log), NULL, log);
      fprintf(stderr, "shader program link error: %s\n", log);
      exit(1);
    }
    glUseProgram(colorRectShaderProg);
    colorLoc = glGetUniformLocation(colorRectShaderProg, "color");
    screenSizeLoc = glGetUniformLocation(colorRectShaderProg, "screen_size");
    originLoc = glGetUniformLocation(colorRectShaderProg, "origin");
    scaleLoc = glGetUniformLocation(colorRectShaderProg, "scale");
    zLoc = glGetUniformLocation(colorRectShaderProg, "z");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, KXGUI_UNIT_QUAD_VERTS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
  }
  glBindVertexArray(vao);
  glUseProgram(colorRectShaderProg);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glUniform2i(scaleLoc, w, h);
  glUniform1i(zLoc, z);
  glUniform4f(colorLoc, red, green, blue, alpha);
  glUniform2i(screenSizeLoc, screenW, screenH);
  glUniform2i(originLoc, x, y);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
};
static void drawTexturedRect(int x, int y, int z, int w, int h, int screenWidth, int screenHeight, unsigned texture, unsigned index) {
  static GLuint
  program = 0,
  vao = 0,
  vbo = 0,
  screenSizeLoc = 0,
  scaleLoc = 0,
  zLoc = 0,
  originLoc = 0,
  textureIndexLoc = 0;
  if (!program) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &rectVertShaderSrc, NULL);
    glCompileShader(vertexShader);
    GLint ok = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(vertexShader, sizeof(log), NULL, log);
      fprintf(stderr, "v shader compile error: %s\n", log);
      exit(1);
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &atlasRectFragShaderSrc, NULL);
    glCompileShader(fragmentShader);
    ok = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(fragmentShader, sizeof(log), NULL, log);
      fprintf(stderr, "f shader compile error: %s\n", log);
      exit(1);
    }
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!ok) {
      char log[512];
      glGetProgramInfoLog(program, sizeof(log), NULL, log);
      fprintf(stderr, "shader program link error: %s\n", log);
      exit(1);
    }
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);
    textureIndexLoc = glGetUniformLocation(program, "textureIndex");
    screenSizeLoc = glGetUniformLocation(program, "screen_size");
    originLoc = glGetUniformLocation(program, "origin");
    scaleLoc = glGetUniformLocation(program, "scale");
    zLoc = glGetUniformLocation(program, "z");
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, KXGUI_UNIT_QUAD_VERTS, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
  }
  glBindVertexArray(vao);
  glUseProgram(program);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
  glUniform1i(zLoc, z);
  glUniform2i(scaleLoc, w, h);
  glUniform2i(originLoc, x, y);
  glUniform2i(screenSizeLoc, screenWidth, screenHeight);
  glUniform1i(textureIndexLoc, index);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
};
static void drawText(const char * text, int x, int y, int z, int screenWidth, int screenHeight, int scaleX, int scaleY) {
  static GLuint
  texture = 0,
  program = 0,
  vao = 0,
  vbo = 0,
  zLoc = 0,
  screenSizeLoc = 0,
  charIndexLoc = 0,
  scaleLoc = 0,
  hasBeenInitialized = 0,
  originLoc = 0;
  if (!hasBeenInitialized) {
    unsigned textureWidth  = KXGUI_CHAR_W * KXGUI_FONT_COLS;
    unsigned textureHeight = KXGUI_CHAR_H * KXGUI_FONT_ROWS;
    unsigned textureSize   = textureWidth * textureHeight;
    unsigned char * fontPixels = malloc(textureSize);
    for (int i = 0; i < textureSize; i++) {
      if (!KXGUI_FONT[i + (int)(i/textureWidth)]) break;
      fontPixels[i] = KXGUI_FONT[i + (int)(i/textureWidth)] == ' ' ? 0x00 : 0xFF;
    }
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R8,textureWidth,textureHeight,0,GL_RED,GL_UNSIGNED_BYTE,fontPixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &glyphVertShaderSrc, NULL);
    glCompileShader(vertexShader);
    GLint ok = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(vertexShader, sizeof(log), NULL, log);
      fprintf(stderr, "v shader compile error: %s\n", log);
      exit(1);
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &glyphFragShaderSrc, NULL);
    glCompileShader(fragmentShader);
    ok = 0;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      char log[512];
      glGetShaderInfoLog(fragmentShader, sizeof(log), NULL, log);
      fprintf(stderr, "f shader compile error: %s\n", log);
      exit(1);
    }
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!ok) {
      char log[512];
      glGetProgramInfoLog(program, sizeof(log), NULL, log);
      fprintf(stderr, "shader program link error: %s\n", log);
      exit(1);
    }
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "tex"), 0); // Tell shader to use unit 0
    glUniform2i(glGetUniformLocation(program, "font_size"), KXGUI_FONT_COLS, KXGUI_FONT_ROWS); // Tell shader to use unit 0
    glUniform2i(glGetUniformLocation(program, "char_size"), KXGUI_CHAR_W, KXGUI_CHAR_H); // Tell shader to use unit 0
    screenSizeLoc =   glGetUniformLocation(program, "screen_size");
    charIndexLoc =    glGetUniformLocation(program, "char_index");
    originLoc =       glGetUniformLocation(program, "origin");
    scaleLoc =        glGetUniformLocation(program, "scale");
    zLoc =            glGetUniformLocation(program, "z");
    // create vao and vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, KXGUI_UNIT_QUAD_VERTS, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    hasBeenInitialized = 1;
  }
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glUseProgram(program);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glUniform2i(scaleLoc, scaleX, scaleY);
  glUniform2i(screenSizeLoc, screenWidth, screenHeight);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  for (int i=0; text[i]; i++) {
    glUniform1i(charIndexLoc, text[i]-32);
    glUniform2i(originLoc, x, y);
    glUniform1i(zLoc, z);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    x += KXGUI_CHAR_W * scaleX;
  }
}
typedef enum {GUI_LAYOUT_MANUAL, GUI_LAYOUT_AUTO} gui_layout;
struct guictx {
  int screenWidth, screenHeight; // screenwidth and screenheight
  int mx, my, mb; // mouse x, mouse y, mouse button boolean
  int textScaleX, textScaleY; // text scaling
  float gui_scale;
  unsigned char typedChar;
  int * keys; // glfw keys
  gui_layout layout;
  unsigned layoutFlags;
  int layoutX, layoutY;
  int rowHeight;
  int top, bottom, left, right;
  int spacing;
};
typedef struct  {
  char * buffer;
  int cursorPos;
  int focused;
} textfield_t;

static void kxguiLayoutFunc(struct guictx * ctx, int * x, int * y, int w, int h) {
  if (ctx->layout == GUI_LAYOUT_AUTO) {
    if (ctx->layoutX + w + ctx->spacing > ctx->right) {
      ctx->layoutX = ctx->left + ctx->spacing;
      ctx->layoutY += ctx->rowHeight + ctx->spacing;
    }
    *x = ctx->layoutX; *y = ctx->layoutY;
    ctx->layoutX += w + ctx->spacing;
    if (h > ctx->rowHeight) ctx->rowHeight = h;
  }
}

void kxguiNewline(struct guictx * ctx) {
  ctx->layoutX = ctx->left + ctx->spacing;
  ctx->layoutY += ctx->rowHeight + ctx->spacing;
  ctx->rowHeight = 0;
}

void textlabel(struct guictx * ctx, char * buffer, int x, int y, char * tooltip) {
  kxguiLayoutFunc(ctx, &x, &y, strlen(buffer) * KXGUI_CHAR_W * ctx->textScaleX, KXGUI_CHAR_H * ctx->textScaleY);
  drawText(buffer, x * ctx->gui_scale, y * ctx->gui_scale, 0, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX * ctx->gui_scale, ctx->textScaleY * ctx->gui_scale);
  if (x * ctx->gui_scale < ctx->mx && ctx->mx < (x+(1+strlen(buffer))*KXGUI_CHAR_W*ctx->textScaleX) * ctx->gui_scale && y * ctx->gui_scale < ctx->my && ctx->my < (y+KXGUI_CHAR_H*ctx->textScaleY) * ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15*ctx->gui_scale,ctx->my+15*ctx->gui_scale, -0.5, ctx->screenWidth,ctx->screenHeight,6*ctx->gui_scale,6*ctx->gui_scale);
  }
}

void textfield(struct guictx * ctx, textfield_t * tf, int x, int y, char * str, int maxLength, char * tooltip) {
  
  static int wasMouseClicked = 0;
  if (!tf->buffer) {
    tf->buffer = (char *) malloc(strlen(str) + 1);
    strcpy(tf->buffer, str);
  }

  kxguiLayoutFunc(ctx, &x, &y, strlen(tf->buffer) * KXGUI_CHAR_W * ctx->textScaleX, KXGUI_CHAR_H * ctx->textScaleY);
  
  drawText(tf->buffer,x*ctx->gui_scale,y*ctx->gui_scale,0,ctx->screenWidth,ctx->screenHeight,ctx->textScaleX * ctx->gui_scale, ctx->textScaleY * ctx->gui_scale);
  drawColorRect(x*ctx->gui_scale+(tf->cursorPos*KXGUI_CHAR_W*ctx->textScaleX)*ctx->gui_scale,y*ctx->gui_scale,0,1*ctx->textScaleX * ctx->gui_scale, KXGUI_CHAR_H*ctx->textScaleY * ctx->gui_scale, ctx->screenWidth, ctx->screenHeight, 1, 1, 1, 1);
  if (tf->focused && ctx->typedChar) { 
    unsigned length = strlen(tf->buffer);
    if (maxLength < 0 ? 0 : length == maxLength) return;
    tf->buffer = (char *) realloc(tf->buffer, length + 2); 
    memmove(tf->buffer + tf->cursorPos + 1, tf->buffer + tf->cursorPos, length + 1 - tf->cursorPos);
    (tf->buffer)[tf->cursorPos] = ctx->typedChar;
    tf->cursorPos = tf->cursorPos + 1;
  }
  if (tf->focused  && ctx->keys[GLFW_KEY_RIGHT] && tf->cursorPos < strlen(tf->buffer) && tf->cursorPos < maxLength ) {
    tf->cursorPos += 1;
    ctx->keys[GLFW_KEY_RIGHT] = 0;
  } 
  if (tf->focused && ctx->keys[GLFW_KEY_LEFT] && tf->cursorPos > 0) {
    tf->cursorPos -= 1;
    ctx->keys[GLFW_KEY_LEFT] = 0;
  }
  if (tf->focused && ctx->keys[GLFW_KEY_BACKSPACE] && tf->cursorPos > 0) {
    tf->cursorPos -= 1;
    ctx->keys[GLFW_KEY_BACKSPACE] = 0;
    unsigned length = strlen(tf->buffer);
    memmove(tf->buffer + tf->cursorPos, tf->buffer + tf->cursorPos + 1, length - tf->cursorPos);
    tf->buffer = (char *) realloc(tf->buffer, length);
  }
  if (x * ctx->gui_scale < ctx->mx && ctx->mx < (x+(1+strlen(tf->buffer))*KXGUI_CHAR_W*ctx->textScaleX) * ctx->gui_scale && y * ctx->gui_scale < ctx->my && ctx->my < (y+KXGUI_CHAR_H*ctx->textScaleY)  * ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15*ctx->gui_scale,ctx->my+15*ctx->gui_scale,-0.5,ctx->screenWidth,ctx->screenHeight,6*ctx->gui_scale,6*ctx->gui_scale);
    if (ctx->mb && !wasMouseClicked) {
      tf->focused = 1;
      tf->cursorPos = (ctx->mx - x * ctx->gui_scale) / ((KXGUI_CHAR_W * ctx->textScaleX) * ctx->gui_scale);
      int len = strlen(tf->buffer);
      if (tf->cursorPos > len) {
        tf->cursorPos = len;
      }
    }
    wasMouseClicked = 0;
    if (ctx->mb) wasMouseClicked = 1;
  } else if (ctx->mb && !wasMouseClicked) {tf->focused  = 0;}
}
// todo get rid of magic numbers
void slider(struct guictx * ctx, int x, int y, double * value, float min, float max, char * tooltip) {
#define KXGUI_SLIDER_W 256
#define KXGUI_SLIDER_H 64
  kxguiLayoutFunc(ctx, &x, &y, KXGUI_SLIDER_W, KXGUI_SLIDER_H);
  drawColorRect(x*ctx->gui_scale,y*ctx->gui_scale,0,KXGUI_SLIDER_W*ctx->gui_scale,KXGUI_SLIDER_H*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  drawColorRect(x*ctx->gui_scale+((*value-min)/(max-min)*KXGUI_SLIDER_W+min)*ctx->gui_scale,y*ctx->gui_scale,-1,KXGUI_SLIDER_H*ctx->gui_scale,KXGUI_SLIDER_H*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,0,0,1);
  if (x*ctx->gui_scale<ctx->mx&&ctx->mx<(x+KXGUI_SLIDER_W)*ctx->gui_scale&&y*ctx->gui_scale<ctx->my&&ctx->my<(y+KXGUI_SLIDER_H)*ctx->gui_scale) {
    char buffer[40];
    sprintf(buffer, "%f %s", *value, tooltip );
    drawColorRect(        ctx->mx*ctx->gui_scale+15,ctx->my*ctx->gui_scale+15,-10, strlen(buffer) * KXGUI_CHAR_W * ctx->textScaleX * ctx->gui_scale, KXGUI_CHAR_H * ctx->textScaleY * ctx->gui_scale, ctx->screenWidth, ctx->screenHeight, 0,0,0,1);
    drawText     (buffer,ctx->mx*ctx->gui_scale+15,ctx->my*ctx->gui_scale+15,-14,ctx->screenWidth,ctx->screenHeight,ctx->textScaleX * ctx->gui_scale,ctx->textScaleY * ctx->gui_scale);
    
    if (ctx->mb) {
      *value = (ctx->mx-x*ctx->gui_scale)/(256*ctx->gui_scale) * (max-min) + min;
    }
  }
}
void colorpicker(struct guictx * ctx, int x, int y, unsigned * value, char * tooltip) {

  kxguiLayoutFunc(ctx, &x, &y, 256, 264);
  
  unsigned char r = (*value >> 24) & 0xFF;
  unsigned char g = (*value >> 16) & 0xFF;
  unsigned char b = (*value >> 8)  & 0xFF;
  unsigned char a = (*value)       & 0xFF;
    
  drawColorRect(x*ctx->gui_scale,y*ctx->gui_scale,0,KXGUI_SLIDER_W*ctx->gui_scale,64*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  drawColorRect((x+r)*ctx->gui_scale,y*ctx->gui_scale,-1,64*ctx->gui_scale,64*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,0,0,1);
  if (x*ctx->gui_scale<=ctx->mx&&ctx->mx<(x+KXGUI_SLIDER_W)*ctx->gui_scale&&y*ctx->gui_scale<ctx->my&&ctx->my<(y+64)*ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,0,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      r = (ctx->mx-x*ctx->gui_scale)/ctx->gui_scale;
    }
  }

  drawColorRect(x*ctx->gui_scale,(y+100)*ctx->gui_scale,0,KXGUI_SLIDER_W*ctx->gui_scale,64*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  drawColorRect((x+g)*ctx->gui_scale,(y+100)*ctx->gui_scale,-1,64*ctx->gui_scale,64*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,1,0,1);
  if (x*ctx->gui_scale<=ctx->mx&&ctx->mx<(x+KXGUI_SLIDER_W)*ctx->gui_scale&&(y+100)*ctx->gui_scale<ctx->my&&ctx->my<(y+64+100)*ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,-16,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      g = (ctx->mx-x*ctx->gui_scale)/ctx->gui_scale;
    }
  }

  drawColorRect(x*ctx->gui_scale,(y+200)*ctx->gui_scale,0,KXGUI_SLIDER_W*ctx->gui_scale,KXGUI_SLIDER_H * ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  drawColorRect((x+b)*ctx->gui_scale,(y+200)*ctx->gui_scale,-1,KXGUI_SLIDER_H*ctx->gui_scale,KXGUI_SLIDER_H * ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,0,1,1);
  if (x*ctx->gui_scale<=ctx->mx&&ctx->mx<(x+256)*ctx->gui_scale&&(y+200)*ctx->gui_scale<ctx->my&&ctx->my<(y+64+200)*ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,-16,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      b = (ctx->mx-x*ctx->gui_scale)/ctx->gui_scale;
    }
  }

  *value = (r << 24) | (g << 16) | (b << 8) | a;
}

void checkbox(struct guictx * ctx, int x, int y, int * value, char * tooltip) {

  kxguiLayoutFunc(ctx, &x, &y, 64, 64);

  static int wasMouseClicked = 0;
  drawColorRect(x * ctx->gui_scale,y * ctx->gui_scale,0,64 * ctx->gui_scale,64 * ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  drawColorRect((x+4)*ctx->gui_scale,(y+4)*ctx->gui_scale,-1,56*ctx->gui_scale,56*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,0,0,1);
  drawColorRect((x+8)*ctx->gui_scale,(y+8)*ctx->gui_scale,-2,48*ctx->gui_scale,48*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
  if (*value != 0) drawColorRect((x+12)*ctx->gui_scale,(y+12)*ctx->gui_scale,-3,40*ctx->gui_scale,40*ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,0,0,1);
  if (x*ctx->gui_scale<ctx->mx&&ctx->mx<(x+64)*ctx->gui_scale&&y*ctx->gui_scale<ctx->my&&ctx->my<(y+64)*ctx->gui_scale) {
    drawText(tooltip,ctx->mx+15*ctx->gui_scale,ctx->my+15*ctx->gui_scale,0,ctx->screenWidth,ctx->screenHeight,6*ctx->gui_scale,6*ctx->gui_scale);
    if (ctx->mb && !wasMouseClicked) {
      *value = !(*value);
    }
    wasMouseClicked = 0;
    if (ctx->mb) {
      wasMouseClicked = 1;
    }
  }
}

void atlasedit(struct guictx * ctx, int x, int y, int w, int h, unsigned * nLayers, unsigned layer, unsigned color, unsigned ** data, unsigned * texture) {

  kxguiLayoutFunc(ctx, &x, &y, w*16, h*16);
  
  if (!(*texture)) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *texture);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
                GL_RGBA8, w, h, *nLayers,
                0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  if (!(*data)) {
    *data = calloc(w * h * (*nLayers), sizeof(unsigned));
  }
  if (layer >= *nLayers) {
    int oldLayers = *nLayers;
    int newLayers = layer + 1;
    // grow CPU buffer
    *data = realloc(*data, (size_t)w * h * newLayers * sizeof(unsigned));
    // zero new layers
    memset((*data) + (size_t)oldLayers * w * h, 0,
           (size_t)(newLayers - oldLayers) * w * h * sizeof(unsigned));
    // grow GPU texture
    glBindTexture(GL_TEXTURE_2D_ARRAY, *texture);
    // must reallocate entire array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0,
                 GL_RGBA8, w, h, newLayers,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, *data);
    *nLayers = newLayers;
  }
  glDisable(GL_DEPTH_TEST);
  drawColorRect(x * ctx->gui_scale,y * ctx->gui_scale,(w*16) * ctx->gui_scale,0,(h*16) * ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,0,0,0,1);
  drawTexturedRect(x * ctx->gui_scale,y * ctx->gui_scale,1,(w*16) * ctx->gui_scale,(h*16) * ctx->gui_scale,ctx->screenWidth,ctx->screenHeight,*texture,layer);
  if (x * ctx->gui_scale < ctx->mx && ctx->mx < (x+w*16) * ctx->gui_scale && y * ctx->gui_scale < ctx->my && ctx->my < (y+h*16) * ctx->gui_scale) {
    if (ctx->mb) {
      int px = (ctx->mx - x) / 16;
      int py = (ctx->my - y) / 16;
      int index = (layer * w * h) + (py * w + px);
      (*data)[index] = color;
      glBindTexture(GL_TEXTURE_2D_ARRAY, *texture);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, *data + layer * w * h);
    }
  }
  glEnable(GL_DEPTH_TEST);
}

bool button(struct guictx * ctx, int x, int y, char * label, char * tooltip) {
  int w = strlen(label) * KXGUI_CHAR_W * ctx->textScaleX;
  int h = KXGUI_CHAR_H * ctx->textScaleY;
  drawColorRect(x, y, 0, w, h, ctx->screenWidth, ctx->screenHeight,0,0,1,1);
  drawText(label, x, y, 0, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
  if (x < ctx->mx && ctx->mx < x+w && y < ctx->my && ctx->my < y+h) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,0,ctx->screenWidth,ctx->screenHeight,6,6);
    if ( ctx->mb ) {
      drawColorRect(x,y,0,w,h,ctx->screenWidth,ctx->screenHeight,1,1,1,1);
      drawText(label,x,y,0,ctx->screenWidth,ctx->screenHeight,ctx->textScaleX,ctx->textScaleY);
      return true;
    }
    return false;
  }
}

static bool button_once(struct guictx * ctx, int x, int y, char * label, char * tooltip) {

  kxguiLayoutFunc(ctx, &x, &y, strlen(label) * KXGUI_CHAR_W * ctx->textScaleX, KXGUI_CHAR_H * ctx->textScaleY);
  
  static int wasMouseClickedOnPreviousFrame = 0;
  int w = strlen(label) * KXGUI_CHAR_W * ctx->textScaleX;
  int h = KXGUI_CHAR_H * ctx->textScaleY;
  drawColorRect(x * ctx->gui_scale, y * ctx->gui_scale, 0, w * ctx->gui_scale, h * ctx->gui_scale, ctx->screenWidth, ctx->screenHeight,0,0,1,1);
  drawText(label, x * ctx->gui_scale, y * ctx->gui_scale,-1, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX * ctx->gui_scale, ctx->textScaleY * ctx->gui_scale);
  if (x * ctx->gui_scale < ctx->mx && ctx->mx < (x+w) * ctx->gui_scale && y * ctx->gui_scale < ctx->my && ctx->my < (y+h) * ctx->gui_scale) {
    drawColorRect(        ctx->mx+15,ctx->my+15,-10, strlen(tooltip) * KXGUI_CHAR_W * ctx->textScaleX * ctx->gui_scale, KXGUI_CHAR_H*ctx->textScaleX*ctx->gui_scale, ctx->screenWidth, ctx->screenHeight, 0,0,0,1);
    drawText     (tooltip,ctx->mx+15*ctx->gui_scale,ctx->my+15 * ctx->gui_scale,-14,ctx->screenWidth,ctx->screenHeight,ctx->textScaleX * ctx->gui_scale,ctx->textScaleY * ctx->gui_scale);
    if ( ctx->mb ) {
      drawColorRect(x * ctx->gui_scale, y * ctx->gui_scale, -2, w * ctx->gui_scale, h * ctx->gui_scale, ctx->screenWidth, ctx->screenHeight,1,1,1,1);
      drawText(label, x * ctx->gui_scale, y * ctx->gui_scale, -3, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX * ctx->gui_scale, ctx->textScaleY * ctx->gui_scale);
      if (!wasMouseClickedOnPreviousFrame) {
        wasMouseClickedOnPreviousFrame = 1;
        return true;
      }
    }
  }
  if (!ctx->mb) {
    wasMouseClickedOnPreviousFrame = 0;
  }
  return false;
}

#endif TEXT_H_