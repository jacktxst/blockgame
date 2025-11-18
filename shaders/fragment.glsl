#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec2 texCoords;
out vec4 frag_color;
uniform sampler2D tex0;
void main() {
    frag_color = texture(tex0, texCoords);
}