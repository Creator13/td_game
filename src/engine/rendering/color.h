#pragma once

namespace graphics
{
    struct color
    {
        float r, g, b, a;

        constexpr color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        constexpr color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
        constexpr color() : r(0), g(0), b(0), a(0) {}
    };
}