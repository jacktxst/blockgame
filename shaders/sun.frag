#version 330 core

in vec2 fragTexCoords;

out vec4 fragColor;

uniform sampler2DArray tex0;

void main() {

    fragColor = vec4(texture(tex0, vec3(fragTexCoords, 18)).rgb, 1.0);
    
}
