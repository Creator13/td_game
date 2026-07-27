#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/lighting.inc.glsl"

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

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
} fragIn;

out vec4 fragColor;

///

struct Surface {
    vec3 normal;
    vec3 viewDir;
    vec3 albedo;
    vec3 specularColor;
    float shininess;
};

vec3 phong(LightSample light, Surface surf) {
    float diff = max(dot(surf.normal, light.dir), 0.0);

    vec3 halfDir = normalize(light.dir + surf.viewDir);
    float spec = pow(max(dot(surf.normal, halfDir), 0.0), surf.shininess);

    vec3 diffuse = diff * surf.albedo;
    vec3 specular = spec * surf.specularColor;

    return (diffuse + specular) * light.radiance;
}

void main() {
    Surface surf;
    surf.normal = normalize(fragIn.vNorm);
    surf.viewDir = normalize(scene.cameraPos - fragIn.vWorldPos);
    surf.albedo = texture(diffuseTexture, fragIn.vTexCoord).rgb * diffuseColor.rgb;
    surf.specularColor = texture(specularTexture, fragIn.vTexCoord).rgb * specularColor.rgb;
    surf.shininess = shininess;

    vec3 litColor = lighting.ambientIntensity * surf.albedo;
    litColor += phong(sampleDirectionalLight(lighting.mainLight), surf);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        litColor += phong(samplePointLight(lighting.pointLights[i], fragIn.vWorldPos), surf);
    }

    fragColor = vec4(litColor, 1.0);
}
