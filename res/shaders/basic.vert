#version 460 core

#include "_FrameDataBlock"
#include "_PerDrawBlock"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNorm;

void main() {
    vec4 worldPos = object.worldTransform * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;
    vPos = aPos;
    vNorm = aNorm;
}