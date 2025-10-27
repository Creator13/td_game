#pragma once

struct Color
{
    float r, g, b, a;

    constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
};
