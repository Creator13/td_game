#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/lighting.inc.glsl"

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture_NORM;

layout (binding = 3, std140) uniform MaterialData {
    vec4 diffuseColor;
    vec4 specularColor;
    float shininess;
};

in VertToFrag {
    vec3 vWorldPos;
    vec3 vLocalPos;
    vec3 vNorm;
    vec2 vTexCoord;
    vec4 vFragPosLightSpace;
    vec4 vTangent;
} fragIn;

out vec4 fragColor;

//##

struct Surface {
    vec3 normal;
    vec3 viewDir;
    vec3 albedo;
    vec3 specularColor;
    float shininess;
};

vec3 blinnPhong(LightSample light, Surface surf) {
    float diff = max(dot(surf.normal, light.dir), 0.0);

    vec3 halfDir = normalize(light.dir + surf.viewDir);
    float spec = pow(max(dot(surf.normal, halfDir), 0.0), surf.shininess);

    vec3 diffuse = diff * surf.albedo;
    vec3 specular = spec * surf.specularColor;

    return (diffuse + specular) * light.radiance;
}

void main() {
    Surface surf;
    surf.viewDir = normalize(scene.cameraPos - fragIn.vWorldPos);
    surf.albedo = texture(diffuseTexture, fragIn.vTexCoord).rgb * diffuseColor.rgb;
    surf.specularColor = texture(specularTexture, fragIn.vTexCoord).rgb * specularColor.rgb;
    surf.shininess = shininess;

    vec3 N = normalize(fragIn.vNorm);
    vec3 T = normalize(fragIn.vTangent.xyz);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T) * fragIn.vTangent.w;
    mat3 TBN = mat3(T, B, N);

    vec3 localNormal = texture(normalTexture_NORM, fragIn.vTexCoord).rgb * 2.0 - 1.0;
    surf.normal = normalize(TBN * localNormal);

    float shadow = sampleShadow(fragIn.vFragPosLightSpace);
    vec3 litColor = lighting.ambientIntensity * surf.albedo;
    // By convention the first dirLight is considered the main (shadow casting) light in the scene
    litColor += shadow * blinnPhong(sampleDirectionalLight(lighting.dirLights[0]), surf);

    for (int i = 1; i < lighting.numDirLights; i++) {
        litColor += blinnPhong(sampleDirectionalLight(lighting.dirLights[i]), surf);
    }

    for (int i = 0; i < lighting.numPointLights; i++) {
        litColor += blinnPhong(samplePointLight(lighting.pointLights[i], fragIn.vWorldPos), surf);
    }

    for (int i = 0; i < lighting.numSpotlights; i++) {
        litColor += blinnPhong(sampleSpotlight(lighting.spotlights[i], fragIn.vWorldPos), surf);
    }

    fragColor = vec4(litColor, 1.0);
}
