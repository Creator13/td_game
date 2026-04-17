#pragma once

#include <string_view>

#include "datatype.h"
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

    struct alignas(16) PassDataBlock
    {
        math::mat4 view;
        math::mat4 projection;
        math::mat4 viewProj;
        float time;

        constexpr static gl::Int SHADER_BINDING = 0;

        constexpr static std::string_view getShaderDeclaration()
        {
            return R"(
layout (binding = 0, std140) uniform PassDataBlock
{
    mat4 view;
    mat4 projection;
    mat4 viewProj;
    float time;
} scene;
            )";
        }
    };

    struct alignas(16) InstanceData
    {
        math::mat4 transform;

        constexpr static gl::Int SHADER_BINDING = 1;

        constexpr static std::string_view getShaderDeclaration()
        {
            return R"(
struct InstanceData {
    mat4 transform;
};

layout (std430, binding = 1) readonly buffer instanceSSBO {
    InstanceData instances[];
};

#define INSTANCE_TRANSFORM (instances[gl_BaseInstance + gl_InstanceID].transform)
            )";
        }
    };
}
