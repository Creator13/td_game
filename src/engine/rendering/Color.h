#pragma once
#include "math/func.h"

namespace graphics
{
    struct Color;

    struct ColorLinear
    {
        float r, g, b, a;

        constexpr ColorLinear() : r(0), g(0), b(0), a(0) { }
        static constexpr ColorLinear fromSrgb(const Color& srgbColor);
        static constexpr ColorLinear fromSrgb(float r, float g, float b, float a);
        static constexpr ColorLinear fromSrgb(float r, float g, float b);
    };

    struct Color
    {
        float r, g, b, a;

        constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { }
        constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }
        constexpr Color() : r(0), g(0), b(0), a(0) { }

        static const Color black, white, red, green, blue, yellow, cyan, magenta;
        static const Color gray1, gray2, gray3, gray4, gray, gray6, gray7, gray8, gray9;
        static const Color lightRed, darkRed, lightGreen, darkGreen, lightBlue, darkBlue;
    };

    // @formatter:off
    inline const Color Color::black     (0.0f, 0.0f, 0.0f);
    inline const Color Color::white     (1.0f, 1.0f, 1.0f);
    inline const Color Color::red       (1.0f, 0.0f, 0.0f);
    inline const Color Color::green     (0.0f, 1.0f, 0.0f);
    inline const Color Color::blue      (0.0f, 0.0f, 1.0f);
    inline const Color Color::yellow    (1.0f, 1.0f, 0.0f);
    inline const Color Color::cyan      (0.0f, 1.0f, 1.0f);
    inline const Color Color::magenta   (1.0f, 0.0f, 1.0f);

    inline const Color Color::gray1     (0.9f, 0.9f, 0.9f);
    inline const Color Color::gray2     (0.8f, 0.8f, 0.8f);
    inline const Color Color::gray3     (0.7f, 0.7f, 0.7f);
    inline const Color Color::gray4     (0.6f, 0.6f, 0.6f);
    inline const Color Color::gray      (0.5f, 0.5f, 0.5f);
    inline const Color Color::gray6     (0.4f, 0.4f, 0.4f);
    inline const Color Color::gray7     (0.3f, 0.3f, 0.3f);
    inline const Color Color::gray8     (0.2f, 0.2f, 0.2f);
    inline const Color Color::gray9     (0.1f, 0.1f, 0.1f);

    inline const Color Color::lightRed  (1.0f, 0.5f, 0.5f);
    inline const Color Color::darkRed   (0.5f, 0.0f, 0.0f);
    inline const Color Color::lightGreen(0.5f, 1.0f, 0.5f);
    inline const Color Color::darkGreen (0.0f, 0.5f, 0.0f);
    inline const Color Color::lightBlue (0.5f, 0.5f, 1.0f);
    inline const Color Color::darkBlue  (0.0f, 0.0f, 0.5f);
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


    constexpr ColorLinear ColorLinear::fromSrgb(const Color& srgbColor)
    {
        ColorLinear result;
        result.r = srgbToLinear(srgbColor.r);
        result.g = srgbToLinear(srgbColor.g);
        result.b = srgbToLinear(srgbColor.b);
        result.a = srgbColor.a;
        return result;
    }

    constexpr ColorLinear ColorLinear::fromSrgb(float r, float g, float b, float a)
    {
        ColorLinear result;
        result.r = srgbToLinear(r);
        result.g = srgbToLinear(g);
        result.b = srgbToLinear(b);
        result.a = a;
        return result;
    }

    constexpr ColorLinear ColorLinear::fromSrgb(float r, float g, float b)
    {
        ColorLinear result;
        result.r = srgbToLinear(r);
        result.g = srgbToLinear(g);
        result.b = srgbToLinear(b);
        result.a = 1.f;
        return result;
    }
}
