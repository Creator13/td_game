#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/instancing.inc.glsl"
#include "cginc/lighting.inc.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec4 aTangent;

out VertToFrag {
    vec3 vWorldPos;
    vec3 vLocalPos;
    vec3 vNorm;
    vec2 vTexCoord;
    vec4 vFragPosLightSpace;
    vec4 vTangent;
} vertOut;

void main() {
    vec4 worldPos = INSTANCE_TRANSFORM * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;

    mat3 modelMat = mat3(INSTANCE_TRANSFORM);
    mat3 normalMat = mat3(transpose(INSTANCE_TRANSFORM_INVERSE));

    vertOut.vWorldPos = worldPos.xyz;
    vertOut.vLocalPos = aPos;
    vertOut.vTexCoord = aTexCoord;
    vertOut.vNorm = normalize(normalMat * aNorm);
//    vertOut.vNorm = normalMat * aNorm;
    vertOut.vTangent = vec4(normalize(modelMat * aTangent.xyz), aTangent.w);

    // shadow normal offset bias
    vec3 L = normalize(-lighting.dirLights[0].direction);
    float NdotL = clamp(dot(vertOut.vNorm, L), 0.0, 1.0);
    float sinTheta = sqrt(1.0 - NdotL * NdotL);

    const float normalBiasTexels = 3;
    float normalOffset = normalBiasTexels * lighting.shadowTexelWorldSize * sinTheta;

    vec3 biasedWorldPos = vertOut.vWorldPos + vertOut.vNorm * normalOffset;
    vertOut.vFragPosLightSpace = lighting.lightSpaceMatrix * vec4(biasedWorldPos, 1.0);
}
