#pragma once
#include "math/mat4.h"

namespace core::gfx
{
    struct alignas(4) Std140Bool
    {
        u32 value;
        constexpr Std140Bool(bool value) : value{value} { }
        constexpr operator bool() const { return value; }
    };

    struct alignas(16) Std140Vec3
    {
        math::vec4 value;
        constexpr Std140Vec3(math::vec3 value) : value(value, 0) { }

        constexpr Std140Vec3 operator=(math::vec3 v)
        {
            value = {v, 0};
            return *this;
        }

        constexpr operator math::vec3() const { return value.xyz(); }
    };

    struct FrameDataBlock
    {
        math::mat4 viewProj;
        float time;

        constexpr static std::string_view getShaderDeclaration()
        {
            return R"(
layout (binding = 0, std140) uniform FrameDataBlock
{
    mat4 viewProj;
    float time;
} scene;
            )";
        }
    };

    struct PerDrawBlock
    {
        math::mat4 model;

        constexpr static std::string_view getShaderDeclaration()
        {
            return R"(
layout (binding = 1, std140) uniform PerDrawBlock
{
    mat4 worldTransform;
} object;
            )";
        }
    };
}
