#version 330 core

#define PI 3.14159

in float brightness;
in vec2 uv;
in vec3 fragPos;
out vec4 fragColor;

uniform sampler2DArray tex0;   // color atlas
uniform isampler1D tex1;       // LUT as 2D integer texture

uniform vec3 lightColor;
uniform vec3 cameraPos;

uniform vec3 fogColor;
uniform float time;
uniform float fogStart;
uniform float fogEnd;

in float faceId;
in float blockType;

void main() {

    float dist = length(fragPos - cameraPos);
    float fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);

    int texIndex = texelFetch(tex1, int(blockType * 6 + faceId), 0).r;
    vec4 texColor = texture(tex0, vec3(uv, texIndex));   
    //texColor = vec4(1.0,1.0,1.0,1.0);
    vec4 baseColor;
    
    switch(1) {
    
        case 0: // minecraft style
            baseColor = vec4(texColor.rgb * (lightColor * brightness), blockType == 6 ? 0.5 : 1.0);
        break;
        case 1: // pixel blocks
            baseColor =      vec4(texture(tex0, vec3((fragPos.x)/16.0,(fragPos.z)/16.0,texIndex)).rgb , 1.0);
        break;
    }
    
    //baseColor = vec4(abs(vec3(fragPos.x, fragPos.y , fragPos.z)) * 0.01, 1.0);
 

	
    fragColor = mix(vec4(fogColor, 1.0), baseColor, fogFactor);
}