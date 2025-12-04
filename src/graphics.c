
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <GL/glew.h>
#include "main.h"

#include <stdarg.h>

#define MAX_SHADER_LENGTH 4096

static GLuint loadAndCompileShaderFromFile (GLenum type, char * filename) {
    char src[MAX_SHADER_LENGTH];
    FILE * fptr;
    fptr = fopen(filename, "r");
    if (!fptr) { fprintf(stderr, "Could not open %s\n", filename); exit(1); }
    //fgets(src, MAX_SHADER_LENGTH, fptr);
    size_t len = fread(src, 1, MAX_SHADER_LENGTH - 1, fptr);
    src[len] = '\0';
    fclose(fptr);
    const char * shSrc = src;
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shSrc, NULL);
    glCompileShader(shader);
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "%s shader compile error: %s\n", type == GL_VERTEX_SHADER ? "vert" : "frag", log);
        exit(1);
    }
    return shader;
}
unsigned shaderProgram(char * vertexPath, char * fragmentPath) {
    GLuint vs = loadAndCompileShaderFromFile(GL_VERTEX_SHADER, vertexPath);
    GLuint fs = loadAndCompileShaderFromFile(GL_FRAGMENT_SHADER, fragmentPath);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(p, sizeof(log), NULL, log);
        fprintf(stderr, "shader program link error: %s\n", log);
        exit(1);
    }
    return p;
}

int drawThing(thing_t * thing) {
    glUseProgram(thing->prog);
    glUniformMatrix4fv(glGetUniformLocation(thing->prog, "model"), 1, GL_FALSE, thing->transform);
    glBindVertexArray(thing->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, thing->tex);
    glUniform1i(glGetUniformLocation(thing->prog, "tex0"), 0);
    
    glDrawElements(GL_TRIANGLES, thing->n, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

opengl_mesh create_opengl_mesh(void * vertices, GLsizeiptr sizeOfVerticesInBytes, unsigned * indices, size_t numIndices, int nAttributes, ...) {
    opengl_mesh m;
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);
    glGenBuffers(1, &m.ebo);
    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeOfVerticesInBytes, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    va_list args;
    va_start(args, nAttributes);
    for (int i = 0; i < nAttributes; i++) {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, va_arg(args, GLint), va_arg(args, GLenum), va_arg(args, GLboolean), va_arg(args, GLsizei), va_arg(args, void*));
    } 
    va_end(args);
    glBindVertexArray(0);
    return m;
}

GLuint createCubeMesh()  {
    static const char vtcs [] = {
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

    static const unsigned idcs [] = {
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

    static const unsigned nV = 24;
    static const unsigned nI = 36;
    
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, nV * sizeof(struct Vertex), vtcs, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, nI * sizeof(unsigned int), idcs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_BYTE, GL_TRUE, sizeof(struct Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_BYTE, GL_TRUE, sizeof(struct Vertex), (void*) offsetof(struct Vertex, i));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(struct Vertex), (void*) offsetof(struct Vertex, u));
    glBindVertexArray(0);
    return VAO;
}