#version 460 core

uniform sampler2D _screenTexture;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = texture(_screenTexture, vTexCoord);
}