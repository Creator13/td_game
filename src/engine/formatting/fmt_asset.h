#pragma once

#include <fmt/format.h>
#include "assets/AssetId.h"

template<>
struct fmt::formatter<core::assets::AssetId> : formatter<uint64_t>
{
    template<typename FormatContext>
    auto format(const core::assets::AssetId& id, FormatContext& ctx) const
    {
        return formatter<uint64_t>::format(id.id, ctx);
    }
};
