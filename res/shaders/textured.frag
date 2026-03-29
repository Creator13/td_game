#version 460 core

#include "_FrameDataBlock"

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

layout (binding = 2, std140) uniform MaterialBlock
{
    vec4 color;
};

uniform sampler2D _mainTex;

out vec4 fragColor;

void main() {
//    vec4 finalColor = color * sin(scene.time);
    fragColor = texture(_mainTex, fragIn.vTexCoord) * color;
//    fragColor = color;
}