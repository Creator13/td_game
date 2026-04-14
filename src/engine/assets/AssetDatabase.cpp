#include "AssetDatabase.h"

#include "assets/File.h"

using namespace core::assets;

std::unique_ptr<AssetDatabase> AssetDatabase::instance = nullptr;

AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : _resourceRootPath(file::getExecutableDir() / resourceRoot) { }

void AssetDatabase::initialize(std::string_view resourceRoot)
{
    ENGINE_ASSERT(instance == nullptr, "AssetDatabase has already been initialized.");

    instance = std::unique_ptr<AssetDatabase>(new AssetDatabase(resourceRoot));
}

void AssetDatabase::destroy()
{
    // Delete the instance pointer, clean up all containing objects
    instance.reset();
}

file::fs::path AssetDatabase::resolveResourcePath(std::string_view path)
{
    ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

    return instance->_resourceRootPath / file::fs::path(path);
}

const AssetInfo& AssetDatabase::getAssetInfo(AssetId id)
{
    ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

    return instance->_registry.at(id);
}

bool AssetDatabase::hasAsset(std::string_view assetName)
{
    ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

    return hasAsset(AssetId::idFromPath(assetName));
}

bool AssetDatabase::hasAsset(AssetId assetId)
{
    ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

    return instance->_registry.contains(assetId);
}
