#version 460 core

#include "cginc/core.inc.glsl"
#include "cginc/instancing.inc.glsl"

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoord;

out VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
    flat uint glyphIndex;
} vertOut;

struct GlyphInstanceData {
    vec2 uvOffset, uvSize;
    vec4 color;
    float fontSize;
};

layout (std430, binding = 3) readonly buffer GlyphBuffer {
    GlyphInstanceData[] glyphs;
};

void main() {
    vec4 worldPos = INSTANCE_TRANSFORM * vec4(aPos, 1.0);
    gl_Position = scene.viewProj * worldPos;

    uint glyphIndex = INSTANCE_DATA.c0;
    GlyphInstanceData glyphData = glyphs[glyphIndex];

    vec2 atlasUv = glyphData.uvOffset + aTexCoord * glyphData.uvSize;

    vertOut.vPos = aPos;
    vertOut.vNorm = aNorm;
    vertOut.vTexCoord = atlasUv;
    vertOut.glyphIndex = glyphIndex;
}