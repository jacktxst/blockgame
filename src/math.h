//
// Created by jack lewis on 11/24/25.
//

#ifndef MATH_H
#define MATH_H

typedef float mat4[16];

typedef struct {int x, y, z;} ivec3;
typedef struct {int x, y;} ivec2;
typedef struct {float x, y, z;} fvec3;
typedef struct {float x, y;} fvec2;
typedef struct {float r, g, b, a;} fcolor_rgba;


fcolor_rgba colorFromUint(unsigned);
void mat4Ortho(mat4 dest, float left, float right, float bottom, float top, float nearZ, float farZ);
void mat4Multiply(mat4 dest, mat4 a, mat4 b);
void mat4Identity(mat4);
void mat4Translate(mat4 dest, float x, float y, float z);
void mat4Scale(mat4 dest, float x, float y, float z);
void mat4Proj(mat4 dest, float fovyRadians, float aspect, float nearZ, float farZ);
void mat4RotateX(mat4 dest, float radians);
void mat4RotateY(mat4 dest, float radians);
void mat4RotateZ(mat4 dest, float radians);

#endif //MATH_H
