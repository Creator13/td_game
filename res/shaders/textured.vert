#version 460 core

#include "_FrameDataBlock"
#include "_PerDrawBlock"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

out VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
} vertOut;

void main() {
    vec4 worldPos = object.worldTransform * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;

    vertOut.vPos = aPos;
    vertOut.vNorm = aNorm;
    vertOut.vTexCoord = aTexCoord;
}