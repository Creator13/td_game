#pragma once

#include <functional>
#include <string_view>
#include <xxhash.h>

#include "datatype.h"

namespace core::assets
{
    struct AssetId
    {
        u64 id;

        constexpr AssetId() : id(0) { }
        constexpr explicit AssetId(u64 id) : id(id) { }

        constexpr bool operator==(const AssetId& other) const = default;
        constexpr bool operator!=(const AssetId& other) const = default;

        constexpr bool operator==(u64 other) const { return id == other; }
        constexpr bool operator!=(u64 other) const { return id != other; }

        constexpr explicit operator u64() const { return id; }

        static AssetId idFromPath(std::string_view path)
        {
            return static_cast<AssetId>(XXH3_64bits(path.data(), path.size()));
        }

        constexpr static AssetId null() { return AssetId(0); }
    };
}

template<>
struct std::hash<core::assets::AssetId>
{
    constexpr size_t operator()(const core::assets::AssetId& a) const noexcept
    {
        return a.id;
    }
};
