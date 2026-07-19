#version 460 core

uniform sampler2D _screenTexture;

in vec2 vTexCoord;
out vec4 outColor;

void main() {
    outColor = vec4(1 - texture(_screenTexture, vTexCoord).rgb, 1);
}
