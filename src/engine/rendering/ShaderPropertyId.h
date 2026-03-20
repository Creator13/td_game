#pragma once

#include <string_view>

#include "datatype.h"

namespace core::gfx
{
    using ShaderPropertyId = u32;
    static constexpr ShaderPropertyId INVALID_PROPERTY_ID = 0;

    constexpr ShaderPropertyId makePropertyId(std::string_view name) noexcept
    {
        uint32_t h = 2166136261u;
        for (const unsigned char c : name)
        {
            h ^= c;
            h *= 16777619u;
        }
        return h ? h : 1u;
    }

    constexpr ShaderPropertyId operator""_spid(const char* name, size_t size) noexcept
    {
        return makePropertyId({name, size});
    }
}
