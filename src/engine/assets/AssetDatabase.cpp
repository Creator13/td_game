#include "AssetDatabase.h"

#include "assets/File.h"

using namespace core::assets;

std::unique_ptr<AssetDatabase> AssetDatabase::instance = nullptr;

AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : _resourceRootPath(file::getExecutableDir() / resourceRoot) { }

void AssetDatabase::initialize(std::string_view resourceRoot)
{
    instance = std::unique_ptr<AssetDatabase>(new AssetDatabase(resourceRoot));
}

void AssetDatabase::destroy()
{
    instance.reset();
}

const AssetInfo& AssetDatabase::getAssetInfo(AssetId assetId)
{
    return instance->_registry.at(assetId);
}

file::fs::path AssetDatabase::resolveResourcePath(std::string_view path)
{
    return instance->_resourceRootPath / file::fs::path(path);
}

bool AssetDatabase::hasAsset(std::string_view assetName)
{
    return hasAsset(AssetId::idFromPath(assetName));
}

bool AssetDatabase::hasAsset(AssetId assetId)
{
    return instance->_registry.contains(assetId);
}
