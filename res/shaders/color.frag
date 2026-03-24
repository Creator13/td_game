#version 460 core

layout (location = 1) in vec3 vPos;

layout (binding = 2, std140) uniform MaterialBlock
{
    vec4 color;
};

out vec4 fragColor;

void main() {
    fragColor = color;
}