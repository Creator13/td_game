#version 460 core

uniform sampler2D _sceneColor;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = vec4(1-texture(_sceneColor, vTexCoord).rgb, 1);
}
