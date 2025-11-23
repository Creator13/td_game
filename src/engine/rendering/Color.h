#pragma once

namespace graphics
{
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
}
