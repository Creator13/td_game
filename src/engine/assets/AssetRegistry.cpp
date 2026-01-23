#include "assets/AssetDatabase.h"
#include "assets/AssetInfo.h"
#include "assets/File.h"

using namespace core;
using namespace core::assets;
using namespace file;

namespace
{
    AssetDatabase* globalAssetDatabase = nullptr;
}

void assets::bindAssetDatabase(AssetDatabase& db)
{
    globalAssetDatabase = &db;
}

fs::path assets::resolveResourcePath(std::string_view path)
{
    return globalAssetDatabase->resolveResourcePath(path);
}

AssetId assets::registerAsset(std::string_view path, AssetType type, void* data)
{
    // globalAssetDatabase->metadata.try_emplace(path, id, type, data);
    return AssetId::idFromPath(path);
}
