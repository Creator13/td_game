#version 460 core

in VertToFrag {
    vec3 vPos;
    vec3 vNorm;
    vec2 vTexCoord;
    flat uint glyphIndex;
} fragIn;

layout (std140, binding = 2) uniform MaterialData {
    float screenPxRange;
};
struct GlyphInstanceData {
    vec2 uvOffset, uvSize;
    vec4 color;
};

layout (std430, binding = 3) readonly buffer GlyphBuffer {
    GlyphInstanceData[] glyphs;
};

uniform sampler2D _msdfAtlas;

out vec4 fragColor;

float median (float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec3 msdf = texture(_msdfAtlas, fragIn.vTexCoord).rgb;
    float sd = median(msdf.r, msdf.g, msdf.b);
    float screenPxDistance = screenPxRange * (sd - 0.5);

    uint idx = fragIn.glyphIndex;
    vec4 color = glyphs[idx].color;

    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    color.a = mix(0, color.a, opacity);
    fragColor = color;
}
