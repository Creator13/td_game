#version 460 core

#include "../cginc/core.inc.glsl"

uniform sampler2D _screenTexture;

in vec2 vTexCoord;
out vec3 outColor;

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75

float rgb2luma(vec3 rgb) {
    return sqrt(dot(rgb, vec3(0.299f, 0.587f, 0.114f)));
}

void main() {
    vec2 inverseScreenSize = 1.0 / frame.screenSize;

    vec3 colorCenter = texture(_screenTexture, vTexCoord).rgb;

    float lumaCenter = rgb2luma(colorCenter);

    float lumaDown = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(0, -1)).rgb);
    float lumaUp = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(0, 1)).rgb);
    float lumaLeft = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(-1, 0)).rgb);
    float lumaRight = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(1, 0)).rgb);

    float lumaMin = min(lumaCenter, min(min(lumaUp, lumaDown), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaUp, lumaDown), max(lumaLeft, lumaRight)));

    float lumaRange = lumaMax - lumaMin;

    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
        outColor = colorCenter;
        return;
    }

    float lumaDownLeft = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(-1, -1)).rgb);
    float lumaUpRight = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(1, 1)).rgb);
    float lumaUpLeft = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(-1, 1)).rgb);
    float lumaDownRight = rgb2luma(textureOffset(_screenTexture, vTexCoord, ivec2(1, -1)).rgb);

    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;

    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpLeft + lumaUpRight;

    float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) + abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 + abs(-2.0 * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) + abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 + abs(-2.0 * lumaDown + lumaDownCorners);

    bool isHorizontal = edgeHorizontal >= edgeVertical;

    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;
    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    bool is1Steepest = abs(gradient1) >= abs(gradient2);

    float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

    float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;
    float lumaLocalAverage = 0;

    if (is1Steepest) {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
    }
    else {
        lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
    }

    vec2 currentUv = vTexCoord;
    if (isHorizontal) {
        currentUv.y += stepLength * 0.5;
    }
    else {
        currentUv.x += stepLength * 0.5;
    }

    vec2 offset = isHorizontal ? vec2(inverseScreenSize.x, 0) : vec2(0, inverseScreenSize.y);
    vec2 uv1 = currentUv - offset;
    vec2 uv2 = currentUv + offset;

    float lumaEnd1 = rgb2luma(texture(_screenTexture, uv1).rgb);
    float lumaEnd2 = rgb2luma(texture(_screenTexture, uv2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    if (!reached1) {
        uv1 -= offset;
    }
    if (!reached2) {
        uv2 += offset;
    }

    if (!reachedBoth) {
        for (int i = 2; i < ITERATIONS; i++) {
            if (!reached1) {
                lumaEnd1 = rgb2luma(texture(_screenTexture, uv1).rgb);
                lumaEnd1 -= lumaLocalAverage;
            }

            if (!reached2) {
                lumaEnd2 = rgb2luma(texture(_screenTexture, uv2).rgb);
                lumaEnd2 -= lumaLocalAverage;
            }

            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            if (!reached1) {
                uv1 -= offset * QUALITY(i);
            }
            if (!reached2) {
                uv2 += offset * QUALITY(i);
            }
            if (reachedBoth) {
                break;
            }
        }
    }

    float distance1 = isHorizontal ? (vTexCoord.x - uv1.x) : (vTexCoord.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - vTexCoord.x) : (uv2.y - vTexCoord.y);

    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    float edgeThickness = distance1 + distance2;

    float pixelOffset = -distanceFinal / edgeThickness + 0.5f;

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0) != isLumaCenterSmaller;
    float finalOffset = correctVariation ? pixelOffset : 0;

    float lumaAverage = (1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
    float subpixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
    float subpixelOffset2 = (-2.0 * subpixelOffset1 + 3.0) * subpixelOffset1 * subpixelOffset1;
    float subpixelOffsetFinal = subpixelOffset2 * subpixelOffset2 * SUBPIXEL_QUALITY;

    finalOffset = max(finalOffset, subpixelOffsetFinal);

    vec2 finalUv = vTexCoord;
    if (isHorizontal) {
        finalUv.y += finalOffset * stepLength;
    }
    else {
        finalUv.x += finalOffset * stepLength;
    }

    outColor = texture(_screenTexture, finalUv).rgb;
}
