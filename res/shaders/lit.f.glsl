#version 460 core

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

out vec3 fragColor;

void main() {
    fragColor = vec3(1, 1, 1);
}
