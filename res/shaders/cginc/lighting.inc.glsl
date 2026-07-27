#version 460 core

#define MAX_LIGHTS 8

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

layout (binding = 4, std140) uniform LightingData {
    DirectionalLight mainLight;
    PointLight pointLights[MAX_LIGHTS];
    int numPointLights;
    float ambientIntensity;
} lighting;

struct LightSample {
    vec3 dir;
    vec3 radiance;
};

LightSample sampleDirectionalLight(DirectionalLight light) {
    LightSample s;
    s.dir = normalize(-light.direction);
    s.radiance = light.color * light.intensity;
    return s;
}

LightSample samplePointLight(PointLight light, vec3 fragPos) {
    vec3 toLight = light.position - fragPos;
    float distSq = dot(toLight, toLight);
    float dist = sqrt(distSq);

    LightSample s;
    s.dir = toLight / dist;
    s.radiance = light.color * light.intensity / max(distSq, 0.0001);
    return s;
}