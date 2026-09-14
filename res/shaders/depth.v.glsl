#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/instancing.inc.glsl"

layout (location = 0) in vec3 aPos;

void main() {
    vec4 worldPos = INSTANCE_TRANSFORM * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;
}