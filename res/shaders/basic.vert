#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

layout (location = 100) uniform mat4 model;
layout (location = 101) uniform mat4 vp_mat;

layout (location = 0) out vec3 vPos;
layout (location = 1) out vec3 vNorm;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = vp_mat * worldPos;
    vPos = aPos;
    vNorm = aNorm;
}