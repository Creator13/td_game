#version 460 core

uniform sampler2D _screenTexture;

layout (binding = 2, std140) uniform MaterialBlock {
    float uExposure;
    float uGammaAdjust;
// TODO implement shader variants and make each mode a separate precompiled variant
    int uMode; // 0 = clamp, 1 = reinhard, 2 = Neutral, 3 = ACES
};

in vec2 vTexCoord;
out vec4 fragColor;

vec3 reinhard(vec3 hdr) {
    return hdr / (hdr + vec3(1.0));
}

vec3 acesFilm(vec3 hdr) {
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;

    return clamp((hdr * (a * hdr + b)) / (hdr * (c * hdr + d) + e), 0.0, 1.0);
}

vec3 clamp01(vec3 hdr) {
    return clamp(hdr, 0.0, 1.0);
}

vec3 hableFilmic(vec3 x) {
    const float a = 0.15, b = 0.50, c = 0.10, d = 0.20, e = 0.02, f = 0.30;
    return ((x * (a * x + c * b) + d * e) / (x * (a * x + b) + d * f)) - e / f;
}

vec3 neutral(vec3 hdr) {
    const float whitePoint = 11.2;
    vec3 current = hableFilmic(hdr);
    vec3 whiteScale = 1.0 / hableFilmic(vec3(whitePoint));
    return current * whiteScale;
}

vec3 linearToSrgb(vec3 lin) {
    bvec3 cutoff = lessThan(lin, vec3(0.0031308));
    vec3 higher = vec3(1.055) * pow(lin, vec3(1.0 / 2.4)) - vec3(0.055);
    vec3 lower = lin * vec3(12.92);
    return mix(higher, lower, cutoff);
}

void main() {
    vec3 hdr = texture(_screenTexture, vTexCoord).rgb * uExposure;

    vec3 mapped;
    switch (uMode) {
        case 1: mapped = reinhard(hdr); break;
        case 2: mapped = neutral(hdr); break;
        case 3: mapped = acesFilm(hdr); break;
        default : mapped = clamp01(hdr);
    }

    vec3 srgb = linearToSrgb(mapped);
    srgb = pow(srgb, vec3(1.0 / uGammaAdjust));

    fragColor = vec4(srgb, 1.0);
}
