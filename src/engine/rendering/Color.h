#pragma once
#include "math/func.h"

namespace graphics
{
    struct SrgbColor;

    struct Color
    {
        float r, g, b, a;

        constexpr Color() : r(0), g(0), b(0), a(0) { }
        static constexpr Color fromSrgb(const SrgbColor& srgbColor);
        static constexpr Color fromSrgb(float r, float g, float b, float a);
        static constexpr Color fromSrgb(float r, float g, float b);
    };

    struct SrgbColor
    {
        float r, g, b, a;

        constexpr SrgbColor(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { }
        constexpr SrgbColor(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }
        constexpr SrgbColor() : r(0), g(0), b(0), a(0) { }

        static const SrgbColor black, white, red, green, blue, yellow, cyan, magenta;
        static const SrgbColor gray1, gray2, gray3, gray4, gray, gray6, gray7, gray8, gray9;
        static const SrgbColor lightRed, darkRed, lightGreen, darkGreen, lightBlue, darkBlue;
    };

    // @formatter:off
    inline const SrgbColor SrgbColor::black     (0.0f, 0.0f, 0.0f);
    inline const SrgbColor SrgbColor::white     (1.0f, 1.0f, 1.0f);
    inline const SrgbColor SrgbColor::red       (1.0f, 0.0f, 0.0f);
    inline const SrgbColor SrgbColor::green     (0.0f, 1.0f, 0.0f);
    inline const SrgbColor SrgbColor::blue      (0.0f, 0.0f, 1.0f);
    inline const SrgbColor SrgbColor::yellow    (1.0f, 1.0f, 0.0f);
    inline const SrgbColor SrgbColor::cyan      (0.0f, 1.0f, 1.0f);
    inline const SrgbColor SrgbColor::magenta   (1.0f, 0.0f, 1.0f);

    inline const SrgbColor SrgbColor::gray1     (0.9f, 0.9f, 0.9f);
    inline const SrgbColor SrgbColor::gray2     (0.8f, 0.8f, 0.8f);
    inline const SrgbColor SrgbColor::gray3     (0.7f, 0.7f, 0.7f);
    inline const SrgbColor SrgbColor::gray4     (0.6f, 0.6f, 0.6f);
    inline const SrgbColor SrgbColor::gray      (0.5f, 0.5f, 0.5f);
    inline const SrgbColor SrgbColor::gray6     (0.4f, 0.4f, 0.4f);
    inline const SrgbColor SrgbColor::gray7     (0.3f, 0.3f, 0.3f);
    inline const SrgbColor SrgbColor::gray8     (0.2f, 0.2f, 0.2f);
    inline const SrgbColor SrgbColor::gray9     (0.1f, 0.1f, 0.1f);

    inline const SrgbColor SrgbColor::lightRed  (1.0f, 0.5f, 0.5f);
    inline const SrgbColor SrgbColor::darkRed   (0.5f, 0.0f, 0.0f);
    inline const SrgbColor SrgbColor::lightGreen(0.5f, 1.0f, 0.5f);
    inline const SrgbColor SrgbColor::darkGreen (0.0f, 0.5f, 0.0f);
    inline const SrgbColor SrgbColor::lightBlue (0.5f, 0.5f, 1.0f);
    inline const SrgbColor SrgbColor::darkBlue  (0.0f, 0.0f, 0.5f);
    // @formatter:on

    constexpr float srgbToLinear(float x)
    {
        if (x <= 0.0f)
            return 0.0f;
        else if (x >= 1.0f)
            return 1.0f;
        else if (x < 0.04045f)
            return x / 12.92f;
        else
            return math::pow((x + 0.055f) / 1.055f, 2.4f);
    }


    constexpr Color Color::fromSrgb(const SrgbColor& srgbColor)
    {
        Color result;
        result.r = srgbToLinear(srgbColor.r);
        result.g = srgbToLinear(srgbColor.g);
        result.b = srgbToLinear(srgbColor.b);
        result.a = srgbColor.a;
        return result;
    }

    constexpr Color Color::fromSrgb(float r, float g, float b, float a)
    {
        Color result;
        result.r = srgbToLinear(r);
        result.g = srgbToLinear(g);
        result.b = srgbToLinear(b);
        result.a = a;
        return result;
    }

    constexpr Color Color::fromSrgb(float r, float g, float b)
    {
        Color result;
        result.r = srgbToLinear(r);
        result.g = srgbToLinear(g);
        result.b = srgbToLinear(b);
        result.a = 1.f;
        return result;
    }
}
