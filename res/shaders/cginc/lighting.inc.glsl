#version 460 core

struct Light {
    vec3 position;
    vec3 color;
    float intensity;
};

layout (binding = 4, std140) uniform LightingData {
    Light light;
    float ambientIntensity;
} lighting;