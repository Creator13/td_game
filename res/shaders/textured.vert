#version 460 core

#include "_FrameDataBlock"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

layout (location = 100) uniform mat4 model;

out VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} vertOut;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;

    vertOut.vPos = aPos;
    vertOut.vNorm = aNorm;
    vertOut.vTexCoord = aTexCoord;
}