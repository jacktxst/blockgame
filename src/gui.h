//
// Created by jack lewis on 11/15/25.
//

// MINIMALISTIC IMMEDIATE MODE SINGLE HEADER GUI LIBRARY FOR C
// I'm pretty happy with this.

// Pure immediate mode GUI system
// Current features
//   drawText
//   drawRect
//   slider
//   checkbox
//   atlasedit
//   colorpicker
//   textfield
//   button
//   button_once
// todo
//  textfield
//   draw multiple lines
//   allow selection
//   blinking cursor
//   click anywhere on it
//  atlasedit
//   multiple layers
//   fill
//   blending
//  z layers
//  textbox
//  dropdown
//  radio button

#ifndef TEXT_H
#define TEXT_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_W 6
#define CHAR_H 6
#define FONT_W 8
#define FONT_H 12

static const char * font =
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
static const char * textVertexShaderSrc =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;         \n"
"uniform ivec2 screen_size;                 \n"
"uniform int   char_index;                  \n"
"uniform ivec2 origin;                      \n"
"uniform ivec2 scale;                       \n"
"uniform ivec2 font_size;                   \n"
"uniform ivec2 char_size;                   \n"
"out     vec2  uv;\n"
"void main() {\n"
"  // Convert integer pixel position to normalized device coords\n"
"  vec2 ndc = vec2(aPos * char_size * scale + origin) / vec2(screen_size) * 2.0 - 1.0;\n"
"  // Flip Y because OpenGL NDC Y+ is up\n"
"  ndc.y = -ndc.y;\n"
"  gl_Position = vec4(ndc, 0.0, 1.0);\n"
"\n"
"  // Compute glyph cell UV in atlas\n"
"  int col = char_index % font_size.x;\n"
"  int row = char_index / font_size.x;\n"
"  vec2 glyphSize = vec2(1.0 / float(font_size.x), 1.0 / float(font_size.y));\n"
"  vec2 glyphUV = vec2(float(col), float(row)) * glyphSize;\n"
"  uv = aPos * glyphSize + vec2(col, row) * glyphSize;\n"
" }\n";
static const char * textFragmentShaderSrc =
"#version 330 core\n"
"in vec2 uv;\n"
"out vec4 fragColor;\n"
"uniform sampler2D tex;\n"
"void main(){\n"
"    float a = texture(tex, uv).r;\n"
"    fragColor = vec4(uv, 1.0, a);\n"
"}\n";
static const char * texRectVertexShaderSrc =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;         \n"
"uniform ivec2 screen_size;                 \n"
"uniform ivec2 origin;                      \n"
"uniform ivec2 scale;                       \n"
"out     vec2  uv;\n"
"void main() {\n"
"  // Convert integer pixel position to normalized device coords\n"
"  vec2 ndc = vec2(aPos * scale + origin) / vec2(screen_size) * 2.0 - 1.0;\n"
"  // Flip Y because OpenGL NDC Y+ is up\n"
"  ndc.y = -ndc.y;\n"
"  gl_Position = vec4(ndc, 0.0, 1.0);\n"
"  uv = aPos;\n"
" }\n";
static const char * texRectFragmentShaderSrc =
"#version 330 core\n"
"in vec2 uv;\n"
"out vec4 fragColor;\n"
"uniform sampler2DArray tex;\n"
"uniform int textureIndex;\n"
"void main(){\n"
"    fragColor = vec4(texture(tex, vec3(uv, textureIndex)));\n"
"}\n";
static const char * rectVertexShaderSrc =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;         \n"
"uniform ivec2 screen_size;                 \n"
"uniform ivec2 origin;                      \n"
"uniform ivec2 scale;                       \n"
"void main() {\n"
"  // Convert integer pixel position to normalized device coords\n"
"  vec2 ndc = vec2(aPos * scale + origin) / vec2(screen_size) * 2.0 - 1.0;\n"
"  // Flip Y because OpenGL NDC Y+ is up\n"
"  ndc.y = -ndc.y;\n"
"  gl_Position = vec4(ndc, 0.0, 1.0);\n"
"}\n";
static const char * rectFragmentShaderSrc =
"#version 330 core\n"
"out vec4 fragColor;\n"
"uniform uint color;\n"
"vec4 uintToRGBA(uint color) {\n"
"     float a = float((color >> 24) & 0xFFu) / 255.0;\n"
"     float r = float((color >> 16) & 0xFFu) / 255.0;\n"
"     float g = float((color >> 8)  & 0xFFu) / 255.0;\n"
"     float b = float(color & 0xFFu) / 255.0;\n"
"     return vec4(r, g, b, a);\n"
"}\n"
"void main(){\n"
"    fragColor = uintToRGBA(color);\n"
"}\n";

void drawRect(int x, int y, int w, int h, int screenWidth, int screenHeight, unsigned color) {
  static GLuint
  program = 0,
  vao = 0,
  vbo = 0,
  screenSizeLoc = 0,
  scaleLoc = 0,
  colorLoc = 0,
  originLoc = 0;
  if (!program) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &rectVertexShaderSrc, NULL);
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
    glShaderSource(fragmentShader, 1, &rectFragmentShaderSrc, NULL);
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
    colorLoc = glGetUniformLocation(program, "color");
    screenSizeLoc = glGetUniformLocation(program, "screen_size");
    originLoc = glGetUniformLocation(program, "origin");
    scaleLoc = glGetUniformLocation(program, "scale");
    // create vao and vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[8] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
  }
  glBindVertexArray(vao);
  glUseProgram(program);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glUniform2i(scaleLoc, w, h);
  glUniform1ui(colorLoc, color);
  glUniform2i(screenSizeLoc, screenWidth, screenHeight);
  glUniform2i(originLoc, x, y);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
};
void drawTexturedRect(int x, int y, int w, int h, int screenWidth, int screenHeight, unsigned texture, unsigned index) {
  static GLuint
  program = 0,
  vao = 0,
  vbo = 0,
  screenSizeLoc = 0,
  scaleLoc = 0,
  originLoc = 0,
  textureIndexLoc = 0;
  if (!program) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &texRectVertexShaderSrc, NULL);
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
    glShaderSource(fragmentShader, 1, &texRectFragmentShaderSrc, NULL);
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
    // create vao and vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[8] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertices, GL_STATIC_DRAW);
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
  glUniform1i(textureIndexLoc, index);
  glUniform2i(scaleLoc, w, h);
  glUniform2i(screenSizeLoc, screenWidth, screenHeight);
  glUniform2i(originLoc, x, y);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
};
void drawText(const char * text, int x, int y, int screenWidth, int screenHeight, int scaleX, int scaleY) {
  static GLuint
  texture = 0,
  program = 0,
  vao = 0,
  vbo = 0,
  screenSizeLoc = 0,
  charIndexLoc = 0,
  scaleLoc = 0,
  originLoc = 0;
  if (!texture) {
    unsigned textureWidth  = CHAR_W * FONT_W;
    unsigned textureHeight = CHAR_H * FONT_H;
    unsigned textureSize   = textureWidth * textureHeight;
    unsigned char * fontPixels = malloc(textureSize);
    for (int i = 0; i < textureSize; i++) {
      if (!font[i + (int)(i/textureWidth)]) break;
      fontPixels[i] = font[i + (int)(i/textureWidth)] == ' ' ? 0x00 : 0xFF;
    }
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(
    GL_TEXTURE_2D,      // target
    0,                  // mip level
    GL_R8,              // internal format: 8-bit single channel (red)
    textureWidth,       // texture width
    textureHeight,      // texture height
    0,                  // border (must be 0)
    GL_RED,             // format of your data: single channel
    GL_UNSIGNED_BYTE,   // type of your data: 1 byte per pixel
    fontPixels          // pointer to the pixel data
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textVertexShaderSrc, NULL);
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
    glShaderSource(fragmentShader, 1, &textFragmentShaderSrc, NULL);
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
    glUniform2i(glGetUniformLocation(program, "font_size"), FONT_W, FONT_H); // Tell shader to use unit 0
    glUniform2i(glGetUniformLocation(program, "char_size"), CHAR_W, CHAR_H); // Tell shader to use unit 0
    screenSizeLoc = glGetUniformLocation(program, "screen_size");
    charIndexLoc = glGetUniformLocation(program, "char_index");
    originLoc = glGetUniformLocation(program, "origin");
    scaleLoc = glGetUniformLocation(program, "scale");
    // create vao and vbo
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    float vertices[8] = {
      0.0f, 0.0f,
      1.0f, 0.0f,
      0.0f, 1.0f,
      1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
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
    
    // this array needs to work for a gl triangle fan
    glUniform1i(charIndexLoc, text[i]-32);
    glUniform2i(originLoc, x, y);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    x += CHAR_W * scaleX;
  }
}

typedef enum {GUI_LAYOUT_MANUAL, GUI_LAYOUT_AUTO} gui_layout;

struct guictx {
  int screenWidth, screenHeight; // screenwidth and screenheight
  int mx, my, mb; // mouse x, mouse y, mouse button boolean
  int textScaleX, textScaleY; // text scaling
  
  unsigned char typedChar;
  int * keys; // glfw keys
  gui_layout layout;
  int layoutX, layoutY;
  int spacing;
};

typedef struct  {
  char * buffer;
  int cursorPos;
  int focused;
} textfield_t;

// textfield_t ipAddrTextfield = {0}
// textfield(&ctx, &ipAddrTextfield, 600, 400, "ip", "ok");
//textfield(&ctx, 600, 400,"ip", &ipBuffer, &cursorPos, &focused, "ok");

void textfield(struct guictx * ctx, textfield_t * tf, int x, int y, char * str, int maxLength, char * tooltip) {
  static int wasMouseClicked = 0;
  if (!tf->buffer) {
    tf->buffer = (char *) malloc(strlen(str) + 1);
    strcpy(tf->buffer, str);
  }
  drawText(tf->buffer, x, y, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
  drawRect(x + (tf->cursorPos * CHAR_W * ctx->textScaleX), y, 1*ctx->textScaleX, CHAR_H*ctx->textScaleY, ctx->screenWidth, ctx->screenHeight, 0xFFFFFFFF);
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
  if (x < ctx->mx && ctx->mx < x+(1+strlen(tf->buffer))*CHAR_W*ctx->textScaleX && y < ctx->my && ctx->my < y+CHAR_H*ctx->textScaleY) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb && !wasMouseClicked) {
      tf->focused = 1;
      tf->cursorPos = (ctx->mx - x) / (CHAR_W * ctx->textScaleX);
      int len = strlen(tf->buffer);
      if (tf->cursorPos > len) {
        tf->cursorPos = len;
      }
    }
    wasMouseClicked = 0;
    if (ctx->mb) wasMouseClicked = 1;
  } else if (ctx->mb && !wasMouseClicked) {tf->focused  = 0;}
}

void slider(struct guictx * ctx, int x, int y, double * value, float min, float max, char * tooltip) {
  drawRect(x,y,256,64,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  drawRect(x+   (    (*value-min)/(max-min)*256+min    ) ,y,64,64,ctx->screenWidth,ctx->screenHeight,0xFF000000);
  if (x < ctx->mx && ctx->mx < x+256 && y < ctx->my && ctx->my < y+64) {
    char buffer[40];
    sprintf(buffer, "%f %s", (ctx->mx-x)/256.0*(max-min)+min, tooltip );
    drawText(buffer,ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      *value = (ctx->mx-x)/256.0*(max-min)+min;
    }
  }
}

void colorpicker(struct guictx * ctx, int x, int y, unsigned * value, char * tooltip) {

  unsigned char a = (*value >> 24) & 0xFF;
  unsigned char b = (*value >> 16) & 0xFF;
  unsigned char g = (*value >> 8)  & 0xFF;
  unsigned char r = (*value)       & 0xFF;
    
  drawRect(x,y,256,64,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  drawRect(x+ r ,y,64,64,ctx->screenWidth,ctx->screenHeight,0xFFFF0000);
  if (x <= ctx->mx && ctx->mx < x+256 && y < ctx->my && ctx->my < y+64) {
    drawText("r",ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      r = ctx->mx-x;
    }
  }

  drawRect(x,y+100,256,64,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  drawRect(x+ g ,y+100,64,64,ctx->screenWidth,ctx->screenHeight,0xFF00FF00);
  if (x <= ctx->mx && ctx->mx < x+256 && y+100 < ctx->my && ctx->my < y+64+100) {
    drawText("g",ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      g = ctx->mx-x;
    }
  }

  drawRect(x,y+200,256,64,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  drawRect(x+ b ,y+200,64,64,ctx->screenWidth,ctx->screenHeight,0xFF0000FF);
  if (x <= ctx->mx && ctx->mx < x+256 && y+200 < ctx->my && ctx->my < y+64+200) {
    drawText("b",ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb) {
      b = ctx->mx-x;
    }
  }

  *value = (a << 24) | (b << 16) | (g << 8) | r;
}

void checkbox(struct guictx * ctx, int x, int y, int * value, char * tooltip) {
  static int wasMouseClicked = 0;
  drawRect(x   ,y,64,64,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  drawRect(x+ 4,y+ 4,56,56,ctx->screenWidth,ctx->screenHeight,0xFF000000);
  drawRect(x+ 8,y+ 8,48,48,ctx->screenWidth,ctx->screenHeight,0xFFFFFFFF);
  if (*value != 0) drawRect(x+12,y+12,40,40,ctx->screenWidth,ctx->screenHeight,0xFF000000);
  if (x < ctx->mx && ctx->mx < x+64 && y < ctx->my && ctx->my < y+64) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if (ctx->mb && !wasMouseClicked) {
      *value = !(*value);
    }
    wasMouseClicked = 0;
    if (ctx->mb) {
      wasMouseClicked = 1;
    }
  }
}

//
void atlasedit(struct guictx * ctx, int x, int y, int w, int h, unsigned * nLayers, unsigned layer, unsigned color, unsigned ** data, unsigned * texture) {
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
  drawRect(x,y,w*16,h*16,ctx->screenWidth,ctx->screenHeight,0xFF000000);
  drawTexturedRect(x,y,w*16,h*16,ctx->screenWidth,ctx->screenHeight,*texture,layer);
  if (x < ctx->mx && ctx->mx < x+w*16 && y < ctx->my && ctx->my < y+h*16) {
    int px = (ctx->mx - x) / 16;
    int py = (ctx->my - y) / 16;
    //char buffer[40];
    //sprintf(buffer, "hi %d %d %08X", px, py, color);
    //drawText(buffer,ctx->mx+15,ctx->my+15,ctx->scrw,ctx->scrh,6,6);
    if (ctx->mb) {
      int px = (ctx->mx - x) / 16;
      int py = (ctx->my - y) / 16;
      int index = (layer * w * h) + (py * w + px);
      (*data)[index] = color;
      glBindTexture(GL_TEXTURE_2D_ARRAY, *texture);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, w, h, 1, GL_RGBA, GL_UNSIGNED_BYTE, *data + layer * w * h);
    }
  }
}

bool button(struct guictx * ctx, int x, int y, char * label, char * tooltip) {
  int w = strlen(label) * CHAR_W * ctx->textScaleX;
  int h = CHAR_H * ctx->textScaleY;
  drawRect(x, y, w, h, ctx->screenWidth, ctx->screenHeight,0xFF0000FF);
  drawText(label, x, y, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
  if (x < ctx->mx && ctx->mx < x+w && y < ctx->my && ctx->my < y+h) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if ( ctx->mb ) {
      drawRect(x, y, w, h, ctx->screenWidth, ctx->screenHeight,0xFFFFFFFF);
      drawText(label, x, y, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
      return true;
    }
    return false;
  }
}

bool button_once(struct guictx * ctx, int x, int y, char * label, char * tooltip) {
  static int wasMouseClickedOnPreviousFrame = 0;
  int w = strlen(label) * CHAR_W * ctx->textScaleX;
  int h = CHAR_H * ctx->textScaleY;
  drawRect(x, y, w, h, ctx->screenWidth, ctx->screenHeight,0xFF0000FF);
  drawText(label, x, y, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
  if (x < ctx->mx && ctx->mx < x+w && y < ctx->my && ctx->my < y+h) {
    drawText(tooltip,ctx->mx+15,ctx->my+15,ctx->screenWidth,ctx->screenHeight,6,6);
    if ( ctx->mb ) {
      drawRect(x, y, w, h, ctx->screenWidth, ctx->screenHeight,0xFFFFFFFF);
      drawText(label, x, y, ctx->screenWidth, ctx->screenHeight, ctx->textScaleX, ctx->textScaleY);
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