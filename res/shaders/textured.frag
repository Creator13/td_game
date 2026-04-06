#version 460 core

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
    float sinTime = sin(scene.time);
    sinTime = sinTime * .5f + .5f;

    fragColor = texture(_mainTex, fragIn.vTexCoord) * mix(vec4(1), color, sinTime);
}