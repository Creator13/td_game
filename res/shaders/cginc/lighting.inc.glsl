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
    float range;
};

struct Spotlight {
    vec3 position;
    vec3 direction;
    float innerCutoff;

    vec3 color;
    float outerCutoff;
    float intensity;
    float range;
};

layout (binding = 4, std140) uniform LightingData {
    DirectionalLight mainLight;
    PointLight pointLights[MAX_LIGHTS];
    Spotlight spotlights[MAX_LIGHTS];
    int numPointLights;
    int numSpotlights;
    float ambientIntensity;
} lighting;

struct LightSample {
    vec3 dir;
    vec3 radiance;
};

float distanceAttenuation(float squareDistance, float lightRange) {
    float squareRange = lightRange * lightRange;

    float factor = clamp(1.0 - (squareDistance * squareDistance) / (squareRange * squareRange), 0.0, 1.0);
    float window = factor * factor;

    float invSquare = 1.0 / max(squareDistance, 0.0001);
    return invSquare * window;
}

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
    s.radiance = light.color * distanceAttenuation(distSq, light.range);
    return s;
}

LightSample sampleSpotlight(Spotlight light, vec3 fragPos) {
    vec3 toLight = light.position - fragPos;
    float distSq = dot(toLight, toLight);
    float dist = sqrt(distSq);
    vec3 dir = toLight / dist;

    float distAttenuation = distanceAttenuation(distSq, light.range);

    float theta = dot(dir, normalize(-light.direction));
    float coneAttenuation = smoothstep(light.outerCutoff, light.innerCutoff, theta);

    LightSample s;
    s.dir = dir;
    s.radiance = light.color * light.intensity * distAttenuation * coneAttenuation;
    return s;
}