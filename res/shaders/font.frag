#version 460 core

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
    flat uint glyphIndex;
} fragIn;

layout (std140, binding = 3) uniform MaterialData {
    float emRange;
};

struct GlyphInstanceData {
    vec2 uvOffset, uvSize;
    vec4 color;
    float fontSize;
};

layout (std430, binding = 3) readonly buffer GlyphBuffer {
    GlyphInstanceData[] glyphs;
};

uniform sampler2D msdfAtlas;

out vec4 fragColor;

float median (float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    GlyphInstanceData glyphData = glyphs[fragIn.glyphIndex];
    float screenPxRange = emRange * glyphData.fontSize;

    vec4 mtsdf = texture(msdfAtlas, fragIn.vTexCoord);
    float msdf_sd = median(mtsdf.r, mtsdf.g, mtsdf.b);
    float true_sd = mtsdf.a;

    float msdfWeight = clamp(screenPxRange - 0.5f, 0, 1.0f);
    float sd = mix(true_sd, msdf_sd, msdfWeight);

    float effectivePxRange = max(screenPxRange, 1.0f);
    float screenPxDistance = effectivePxRange * (sd - 0.5f);

    float opacity = clamp(screenPxDistance + 0.5f, 0.0, 1.0f);

    vec4 color = glyphData.color;
    fragColor = color * opacity;
}
