#pragma once
#include <filesystem>

#include "assets/AssetId.h"
#include "assets/AssetTraits.h"

namespace core
{
    struct Mesh;
}

namespace core::assets
{
    class AssetDatabase;
    struct AssetInfo;

    void bindAssetDatabase(AssetDatabase& db);
    std::filesystem::path resolveResourcePath(std::string_view path);

    AssetId registerAsset(std::string_view path, AssetType type, void* data);
    const AssetInfo& getAssetInfo(AssetId id);
    bool assetExists(AssetId id);
}
