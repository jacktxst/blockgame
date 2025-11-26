#version 330 core

in float brightness;
in vec2 uv;
out vec4 fragColor;

uniform sampler2DArray tex0;   // color atlas
uniform isampler1D tex1;       // LUT as 2D integer texture

in float faceId;
in float blockType;

void main() {
    int texIndex = texelFetch(tex1, int(blockType * 6 + faceId), 0).r;
    vec4 texColor = texture(tex0, vec3(uv, texIndex));
    fragColor = vec4(texColor.rgb * brightness, 1.0);
}
