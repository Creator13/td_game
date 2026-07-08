#version 460 core

uniform sampler2D _sceneColor;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = texture(_sceneColor, vTexCoord);
}