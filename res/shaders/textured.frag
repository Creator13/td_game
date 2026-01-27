#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vTexCoord;

layout (location = 110) uniform sampler2D texSampler;

out vec4 fragColor;

void main() {
    fragColor = texture(texSampler, vTexCoord);
//    fragColor = vec4(vTexCoord, 1.0, 1.0);
}