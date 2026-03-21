#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vTexCoord;

layout (binding = 1, std140) uniform MaterialBlock
{
    vec4 colorrr;
    float someVal;
};

uniform sampler2D _mainTex;

out vec4 fragColor;

void main() {
    vec4 realColor = colorrr * someVal;
    fragColor = texture(_mainTex, vTexCoord) * realColor;
}