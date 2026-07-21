#version 460 core

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

layout (binding = 2, std140) uniform MaterialBlock
{
    vec4 albedoColor;
};

uniform sampler2D _mainTex;

out vec4 fragColor;

void main() {
    fragColor = texture(_mainTex, fragIn.vTexCoord) * albedoColor;
}