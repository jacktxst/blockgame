//
// Created by jack lewis on 10/27/25.
//

#include <math.h>
#include <stdio.h>
#include "main.h"
#include <stdint.h>

void mat4Identity(mat4 dest) {
    const static float m[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    for (int i = 0; i < 16; i++) {
        dest[i] = m[i];
    }
}
void mat4Ortho(mat4 dest, float left, float right, float bottom, float top, float nearZ, float farZ) {
    for (int i = 0; i < 16; i++) dest[i] = 0.0f;
    float rightMinusLeft = right - left;
    float topMinusBottom = top - bottom;
    float farZminusNearZ = farZ - nearZ;
    dest[0]  = 2.0f / (rightMinusLeft);
    dest[5]  = 2.0f / (topMinusBottom);
    dest[10] = -2.0f / (farZminusNearZ);
    dest[12] = - (right + left) / (rightMinusLeft);
    dest[13] = - (top + bottom) / (topMinusBottom);
    dest[14] = - (farZminusNearZ) / (farZminusNearZ);
    dest[15] = 1.0f;
}
void mat4Multiply(mat4 dest, mat4 a, mat4 b) {
    for (int i = 0; i < 16; i++) { dest[i] = 0.0f; }
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            dest[col*4 + row] =
                a[0*4 + row] * b[col*4 + 0] +
                a[1*4 + row] * b[col*4 + 1] +
                a[2*4 + row] * b[col*4 + 2] +
                a[3*4 + row] * b[col*4 + 3];
        }
    }
}
void mat4Translate(mat4 dest, float x, float y, float z) {
    mat4Identity(dest);
    dest[12] = x;
    dest[13] = y;
    dest[14] = z;
}
void mat4Scale(mat4 dest, float x, float y, float z) {
    mat4Identity(dest);
    dest[0]  = x;
    dest[5]  = y;
    dest[10] = z;
}
void mat4Proj(mat4 dest, float fovyRadians, float aspect, float nearZ, float farZ) {
    float f = 1.0f / tanf(fovyRadians / 2.0f);
    for (int i = 0; i < 16; i++) { dest[i] = 0.0f; }
    dest[0]  = f / aspect;
    dest[5]  = f;
    dest[10] = (farZ + nearZ) / (nearZ - farZ);
    dest[11] = -1.0f;
    dest[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
}
void mat4RotateX(mat4 dest, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat4Identity(dest);
    dest[5]  = c;
    dest[6]  = s;
    dest[9]  = -s;
    dest[10] = c;
}
void mat4RotateY(mat4 dest, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat4Identity(dest);
    dest[0]  = c;
    dest[2]  = -s;
    dest[8]  = s;
    dest[10] = c;
}

/* perlin noise data and functions. llm generated code.
 * likely need optimization.
 */
static const int perm[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
    140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
    247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
    57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
    60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
    65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,
    196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,
    217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,
    206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,
    248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,
    39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
    97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,
    204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    // repeat the table
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
    140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
    247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
    57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
    60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
    65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,
    196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,
    217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,
    206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,
    248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,
    39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
    97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,
    204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180
};
static float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static float lerp(float a, float b, float t) { return a + t * (b - a); }
static float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h<8 ? x : y;
    float v = h<4 ? y : (h==12||h==14 ? x : z);
    return ((h&1)?-u:u) + ((h&2)?-v:v);
}

float perlin3d(float x, float y, float z) {
    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;
    int Z = (int)floorf(z) & 255;

    x -= floorf(x); y -= floorf(y); z -= floorf(z);
    float u = fade(x), v = fade(y), w = fade(z);

    int A  = perm[X] + Y;
    int AA = perm[A] + Z;
    int AB = perm[A + 1] + Z;
    int B  = perm[X + 1] + Y;
    int BA = perm[B] + Z;
    int BB = perm[B + 1] + Z;

    return lerp(
        lerp(
            lerp(grad(perm[AA], x, y, z), grad(perm[BA], x-1, y, z), u),
            lerp(grad(perm[AB], x, y-1, z), grad(perm[BB], x-1, y-1, z), u),
            v
        ),
        lerp(
            lerp(grad(perm[AA+1], x, y, z-1), grad(perm[BA+1], x-1, y, z-1), u),
            lerp(grad(perm[AB+1], x, y-1, z-1), grad(perm[BB+1], x-1, y-1, z-1), u),
            v
        ),
        w
    );
}
// --- octave version for richer blobs ---
float perlin3d_octaves(float x, float y, float z, int octaves, float persistence) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += perlin3d(x * frequency, y * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }
    return total / maxValue; // normalize to [-1,1] approximately
}
// --- binary version ---
int perlin3d_binary(float x, float y, float z, float threshold) {
    float n = perlin3d_octaves(x, y, z, 4, 0.5f); // 4 octaves, persistence 0.5
    //printf(".\n");
    return n > threshold ? 1 : 0;
}

#include <math.h>

// --- permutation table (same as your 3D version) ---
static const int perm2d[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
    140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
    247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
    57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
    60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
    65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,
    196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,
    217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,
    206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,
    248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,
    39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
    97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,
    204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    // repeat the table
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
    140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
    247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
    57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
    74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
    60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
    65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,
    196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,
    217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,
    206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,
    248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,
    39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,
    97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,
    145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,
    204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,
    67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

// --- helper functions ---
static float fade2d(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static float lerp2d(float a, float b, float t) { return a + t * (b - a); }

static const int grad2_table[8][2] = {
    { 1, 0}, {-1, 0}, {0, 1}, {0,-1},
    { 1, 1}, {-1, 1}, {1,-1}, {-1,-1}
};

static float grad2d(int hash, float x, float y) {
    int h = hash & 7;
    float gx = (float)grad2_table[h][0];
    float gy = (float)grad2_table[h][1];
    return gx * x + gy * y;
}


// --- 2D Perlin noise ---
float perlin2d(float x, float y) {
    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;

    x -= floorf(x);
    y -= floorf(y);

    float u = fade2d(x);
    float v = fade2d(y);

    int A = perm2d[X] + Y;
    int B = perm2d[X + 1] + Y;

    return lerp2d(
        lerp2d(grad2d(perm2d[A], x, y), grad2d(perm2d[B], x - 1, y), u),
        lerp2d(grad2d(perm2d[A + 1], x, y - 1), grad2d(perm2d[B + 1], x - 1, y - 1), u),
        v
    );
}

// --- octave version for richer noise ---
float perlin2d_octaves(float x, float y, int octaves, float persistence) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += perlin2d(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue; // normalized
}

// --- binary threshold version ---
int perlin2d_binary(float x, float y, float threshold) {
    float n = perlin2d_octaves(x, y, 4, 0.5f); // 4 octaves, persistence 0.5
    return n > threshold ? 1 : 0;
}

fcolor_rgba colorFromUint(unsigned color) {
    fcolor_rgba rgba;
    rgba.r = ((color >> 24) & 0xFF) / 255.0;
    rgba.g = ((color >> 16) & 0xFF) / 255.0;
    rgba.b = ((color >> 8)  & 0xFF) / 255.0;   
    rgba.a = (color & 0xFF) / 255.0;
    return rgba;
}

// llm generated
void updateProceduralTexture(uint32_t * pixels, int width, int height, int fc) {
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            uint8_t r = (x + fc) % 256;
            uint8_t g = (y + fc) % 256;
            uint8_t b = ((x ^ y) + fc) % 256;
            pixels[y * width + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }
}



