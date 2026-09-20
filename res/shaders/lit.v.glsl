#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/instancing.inc.glsl"
#include "cginc/lighting.inc.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

out VertToFrag {
    vec3 vWorldPos;
    vec3 vLocalPos;
    vec3 vNorm;
    vec2 vTexCoord;
    vec4 vFragPosLightSpace;
} vertOut;

void main() {
    vec4 worldPos = INSTANCE_TRANSFORM * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;

    vertOut.vWorldPos = worldPos.xyz;
    vertOut.vLocalPos = aPos;
    vertOut.vNorm = mat3(transpose(INSTANCE_TRANSFORM_INVERSE)) * aNorm;
    vertOut.vTexCoord = aTexCoord;

    // shadow normal offset bias
    float NdotL = clamp(dot(vertOut.vNorm, normalize(-lighting.dirLights[0].direction)), 0.0, 1.0);
    float normalOffsetScale = 0.33 * (1.0 - NdotL);
    vec3 biasedWorldPos = vertOut.vWorldPos + (vertOut.vNorm * normalOffsetScale);

    vertOut.vFragPosLightSpace = lighting.lightSpaceMatrix * vec4(biasedWorldPos, 1.0);
}
