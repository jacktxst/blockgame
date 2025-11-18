#version 330 core

// Input vertex attributes (read from the vertex buffer)
layout(location = 0) in vec3 aPos;       // The position of the vertex
layout(location = 1) in vec3 aNormal;    // The normal vector for lighting
layout(location = 2) in vec2 aTexCoords; // Texture coordinates

// Uniform variables (read-only data passed from the CPU)
uniform mat4 model;       // Model matrix
uniform mat4 view;        // View matrix
uniform mat4 projection;  // Projection matrix

// Output variables (sent to the fragment shader)
out vec3 fragPos;         // Position of the vertex in world space
out vec3 normal;          // Normal vector in world space
out vec2 texCoords;       // Texture coordinates

void main() {
    // Transform vertex position to clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Calculate position and normal in world space to pass to the fragment shader
    fragPos = vec3(model * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates to the fragment shader
    texCoords = aTexCoords;
}
