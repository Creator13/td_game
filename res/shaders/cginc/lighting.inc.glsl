#version 460 core

layout (binding = 4, std140) uniform LightingData {
    vec3 lightColor;
    vec3 lightPos;
    float ambientStrength;
} lighting;