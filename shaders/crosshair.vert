#version 330 core

// Input vertex attributes (read from the vertex buffer)
layout(location = 0) in vec3 aPos;       // The position of the vertex

// Uniform variables (read-only data passed from the CPU)
uniform mat4 model;       // Model matrix

void main() {
    // Transform vertex position to clip space
    gl_Position = model * vec4(aPos, 1.0);
}
