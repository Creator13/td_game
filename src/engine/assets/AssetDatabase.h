#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "assets/AssetId.h"
#include "assets/AssetInfo.h"
#include "assets/AssetTraits.h"

namespace fastgltf
{
    class Parser;
}

namespace core::assets
{
    class AssetDatabase
    {
    public:
        AssetDatabase() = delete;
        explicit AssetDatabase(std::string_view resourceRoot = "res");
        ~AssetDatabase() = default;

        std::filesystem::path resolveResourcePath(std::string_view path) const;

        const AssetInfo& getAssetInfo(AssetId id);

    private:
        std::unordered_map<AssetId, AssetInfo> metadata;
        std::filesystem::path _rootPath;

        static std::string makeInternalPath(AssetType type, std::string_view name);
    };
}
