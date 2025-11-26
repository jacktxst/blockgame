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
out float faceId;
out vec2  uv;

void main() {

    vec3 lightDir = normalize(-1 * vec3(-0.5, -1, -0.25));
    vec3 surfaceNorm;
    float dirLightAmt = 0.8;
    float ambientLight = 0.4;

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
            surfaceNorm = vec3(1.0, 0.0, 0.0);
            faceId = 0.0;
            break;
        case 1: // xneg
            surfaceNorm = vec3(-1.0, 0.0, 0.0);
            faceId = 1.0;
            break;
        case 2: // ypos
            surfaceNorm = vec3(0.0, 1.0, 0.0);
            faceId = 2.0;
            break;
        case 3: // yneg
            surfaceNorm = vec3(0.0, -1.0, 0.0);
            faceId = 3.0;
            break;
        case 4: // zpos
            surfaceNorm = vec3(0.0, 0.0, 1.0);
            faceId = 4.0;
            break;
        default: // zneg
            surfaceNorm = vec3(0.0, 0.0, -1.0);
            faceId = 5.0;
            break;
    }
    
    float diffuse = max(dot(surfaceNorm, lightDir), 0.0);
    brightness = ambientLight + dirLightAmt * diffuse;
}
