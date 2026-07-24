#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/lighting.inc.glsl"

uniform sampler2D baseTexture;

layout (binding = 3, std140) uniform MaterialData {
    vec4 baseColor;
    float specularStrength;
};

in VertToFrag {
    vec3 vWorldPos;
    vec3 vLocalPos;
    vec3 vNorm;
    vec2 vTexCoord;
} fragIn;

out vec4 fragColor;

void main() {
    vec3 ambient = lighting.ambientStrength * lighting.lightColor;

    vec3 fragNormal = normalize(fragIn.vNorm);
    vec3 lightDir = normalize(lighting.lightPos - fragIn.vWorldPos);

    float diff = max(dot(fragNormal, lightDir), 0.0);
    vec3 diffuse = diff * lighting.lightColor;

    vec3 viewDir = normalize(scene.cameraPos - fragIn.vWorldPos);
    vec3 reflectDir = reflect(-lightDir, fragNormal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lighting.lightColor;

    vec3 lighting = ambient + diffuse + specular;
    vec3 albedo = texture(baseTexture, fragIn.vTexCoord).rgb * baseColor.rgb;
    vec3 result = lighting * albedo;
    fragColor = vec4(result, 1.0);
}
