#version 460 core

layout (binding = 0, std140) uniform ViewportDataBlock
{
    mat4 view;
    mat4 projection;
    mat4 viewProj;
} scene;

layout (binding = 1, std140) uniform FrameDataBlock {
    vec2 screenSize;
    float currentTime;
} frame;