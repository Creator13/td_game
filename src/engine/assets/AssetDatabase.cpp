#include "AssetDatabase.h"

#include "Logging.h"
#include "assets/File.h"
#include "core/Assert.h"

namespace fs = std::filesystem;

using namespace core;
using namespace core::assets;

namespace
{
    constexpr std::string_view RUNTIME_PATH = "@runtime";
    constexpr std::string_view INTERNAL_PATH = "@internal";
}

AssetDatabase::AssetDatabase(std::string_view resourceRoot)
    : _rootPath(file::getExecutableDir() / resourceRoot) { }

const AssetInfo& AssetDatabase::getAssetInfo(AssetId id)
{
    ENGINE_ASSERT(metadata.contains(id), "Asset not found: {}", id);
    return metadata.at(id);
}

fs::path AssetDatabase::resolveResourcePath(std::string_view path) const
{
    return _rootPath / fs::path(path);
}

std::string AssetDatabase::makeInternalPath(AssetType type, std::string_view name)
{
    // std::string_view typePath = mapAssetTypeName(type);
    // return fmt::format("{}/{}/{}", INTERNAL_PATH, typePath, name);
    return "";
}