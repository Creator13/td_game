#include "AssetDatabase.h"

#include "assets/File.h"

using namespace core;
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

template<C_AssetType T, size_t PageSize>
PagedStorageStats getPagedStorageStats(const util::PagedStorage<T, PageSize>& storage)
{
    return PagedStorageStats {
        .itemCount = static_cast<u16>(storage.count_alive()),
        .pageCount = static_cast<u16>(storage.page_count()),
        .bytesUsed = static_cast<u32>(storage.mem_size()),
        .assetBytesUsed = static_cast<u32>(storage.count_alive() * sizeof(T)),
        .fragmentation = storage.fragmentation(),
        .occupation = storage.occupation()
    };
}

AssetStats AssetDatabase::stats()
{
    ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

    u32 count = 0;
    u64 bytes = 0;

    const auto pipelineStats = getPagedStorageStats(instance->_pipelineStorage);
    count += pipelineStats.itemCount;
    bytes += pipelineStats.bytesUsed;

    const auto materialStats = getPagedStorageStats(instance->_materialStorage);
    count += materialStats.itemCount;
    bytes += materialStats.bytesUsed;

    const auto textureStats = getPagedStorageStats(instance->_textureStorage);
    count += textureStats.itemCount;
    bytes += textureStats.bytesUsed;

    const auto meshStats = getPagedStorageStats(instance->_meshStorage);
    count += meshStats.itemCount;
    bytes += meshStats.bytesUsed;

    const auto fontStats = getPagedStorageStats(instance->_fontStorage);
    count += fontStats.itemCount;
    bytes += fontStats.bytesUsed;

    return AssetStats {
        .totalBytesUsed = bytes,
        .totalAssetCount =count,
        .pipelineStats = pipelineStats,
        .materialStats = materialStats,
        .textureStats = textureStats,
        .meshStats = meshStats,
        .fontStats = fontStats,
    };
}
