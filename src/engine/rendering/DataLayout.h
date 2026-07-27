#pragma once

#include <string_view>

#include "datatype.h"
#include "math/mat4.h"

namespace core::gfx
{
    struct alignas(4) Std140Bool
    {
        u32 value;
        constexpr Std140Bool() = default;
        constexpr Std140Bool(bool value) : value{value} { }
        constexpr operator bool() const { return value; }
    };

    struct alignas(16) Std140Vec3
    {
        math::vec4 value;
        constexpr Std140Vec3() = default;
        constexpr Std140Vec3(math::vec3 value) : value(value, 0) { }

        constexpr Std140Vec3 operator=(math::vec3 v)
        {
            value = {v, 0};
            return *this;
        }

        constexpr operator math::vec3() const { return value.xyz(); }
    };

    struct alignas(16) ViewportDataBlock
    {
        math::mat4 view = math::mat4::identity;
        math::mat4 projection = math::mat4::identity;
        math::mat4 viewProj = math::mat4::identity;
        Std140Vec3 cameraPos = math::vec3::zero;

        constexpr static gl::Int SHADER_BINDING = 0;

        constexpr static bool isBlockName(std::string_view name)
        {
            return name == "ViewportDataBlock" || name == "ViewportData";
        }
    };

    struct alignas(16) FrameDataBlock
    {
        math::vec2 screenSize;
        float time;

        constexpr static gl::Int SHADER_BINDING = 1;
    };

    struct alignas(16) InstanceData
    {
        struct alignas(16) CustomData
        {
            u32 c0 = 0, c1 = 0, c2 = 0, c3 = 0;
        };

        math::mat4 transform = math::mat4::identity;
        math::mat4 invTransform = math::mat4::identity;
        CustomData customData;

        constexpr static gl::Int SHADER_BINDING = 2;
    };

    struct MaterialBlock
    {
        constexpr static gl::Int SHADER_BINDING = 3;

        constexpr static bool isBlockName(std::string_view name)
        {
            return name == "MaterialBlock" || name == "MaterialDataBlock" || name == "MaterialData" || name == "Material";
        }
    };

    struct alignas(16) LightingDataBlock
    {
        struct alignas(16) PointLight
        {
            Std140Vec3 position;
            math::vec3 color;
            float intensity;
            float range;
        };

        struct alignas(16) DirectionalLight
        {
            Std140Vec3 direction;
            math::vec3 color;
            float intensity;
        };

        struct alignas(16) Spotlight
        {
            Std140Vec3 position; // 4 byte
            math::vec3 direction; // 3 + 1 byte
            float innerCutoff; // 3 + 1 byte

            math::vec3 color;
            float outerCutoff;
            float intensity;
            float range;
        };

        DirectionalLight dirLight;
        PointLight pointLights[8];
        Spotlight spotlights[8];
        i32 numPointLights = 0;
        i32 numSpotlights = 0;
        float ambientStrength;

        constexpr static gl::Int SHADER_BINDING = 4;
    };
}
