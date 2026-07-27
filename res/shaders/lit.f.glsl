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

void main() {
    vec3 fragNormal = normalize(fragIn.vNorm);

    vec3 diffTexSample = texture(diffuseTexture, fragIn.vTexCoord).rgb * diffuseColor.rgb;
    vec3 specTexSample = texture(specularTexture, fragIn.vTexCoord).rgb * specularColor.rgb;

    // Ambient
    vec3 ambient = lighting.ambientIntensity * diffTexSample;

    // Diffuse
    vec3 lightDir = normalize(lighting.light.position - fragIn.vWorldPos);
    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = lighting.light.color * lighting.light.intensity * (diff * diffTexSample);

    // Specular
    vec3 viewDir = normalize(scene.cameraPos - fragIn.vWorldPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lighting.light.color * lighting.light.intensity * (spec * specTexSample);

    vec3 lighting = ambient + diffuse + specular;
    vec3 result = lighting;
    fragColor = vec4(result, 1.0);
}
