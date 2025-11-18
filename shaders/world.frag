#version 330 core
in float brightness;
in vec2 uv;
out vec4 fragColor;
uniform sampler2DArray tex;
in float blockType;
void main(){
   vec4 texColor = texture(tex, vec3(uv, blockType));
   fragColor = vec4(vec3(texColor.r, texColor.g, texColor.b)*brightness,1.0);
}