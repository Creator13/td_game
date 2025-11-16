#version 460 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

out vec4 fragColor;

void main() {
    fragColor = vec4(normalize(vNorm) * .5 + .5, 1.f);
}