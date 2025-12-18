#pragma once

#include <string_view>
#include <xxhash.h>

namespace assets
{
    typedef uint64_t AssetId;

    inline AssetId idFromPath(std::string_view path)
    {
        return static_cast<AssetId>(XXH3_64bits(path.data(), path.size()));
    }
}
