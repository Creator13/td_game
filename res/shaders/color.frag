#version 460 core

layout (location = 1) in vec3 vPos;

layout (location = 50) uniform vec4 _color;

out vec4 fragColor;

void main() {
    fragColor = _color;
}