#version 330 core

// Input vertex attributes (read from the vertex buffer)
layout(location = 0) in vec3 aPos;       
layout(location = 1) in int aCode;    
layout(location = 2) in int aBlockType;    

uniform mat4 model;       // Model matrix
uniform mat4 view;        // View matrix
uniform mat4 projection;  // Projection matrix

out float brightness;          
out float blockType;
out vec2  uv;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    blockType = aBlockType;
    switch (aCode % 6) {
        case 0:
            uv = vec2(0,1);
            break;
        case 1:
            uv = vec2(0,0);
            break;
        case 2:
            uv = vec2(1,1);
            break;
        case 3:
            uv = vec2(1,0);
            break;
    }
    switch (int(aCode / 6)) {
        case 0: // xpos
            brightness = 0.125;
            break;
        case 1: // xneg
            brightness = 0.75;
            break;
        case 2: // ypos
            brightness = 1.0;
            break;
        case 3: // yneg
            brightness = 0.125;
            break;
        case 4: // zpos
            brightness = 0.5;
            break;
        default: // zneg
            brightness = 0.25;
            break;
    }
}
