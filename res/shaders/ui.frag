#version 460 core

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

out vec4 fragColor;

uniform sampler2D _texture;

void main() {
    fragColor = texture(_texture, fragIn.vTexCoord);
}
