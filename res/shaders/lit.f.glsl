#version 460 core

#include "cginc/lighting.inc.glsl"

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

out vec4 fragColor;

void main() {
    fragColor = vec4(1);
}
