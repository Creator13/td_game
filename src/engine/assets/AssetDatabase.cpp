#include "AssetDatabase.h"

#include <xxhash.h>
#include <spdlog/fmt/fmt.h>

#include "rendering/Mesh.h"

constexpr std::string_view RUNTIME_PATH = "@runtime/";

assets::AssetId assets::AssetDatabase::idFromPath(std::string_view path)
{
    return static_cast<AssetId>(XXH3_64bits(path.data(), path.size()));
}

assets::AssetInfo assets::AssetDatabase::getAssetInfo(AssetId id)
{
    assert(metadata.contains(id));
    // Returns a copy; no outside influence is allowed
    return metadata.at(id);
}

graphics::Mesh& assets::AssetDatabase::getMesh(AssetId id)
{
    assert(metadata.contains(id));
    return meshes.at(id);
}

void assets::AssetDatabase::loadMesh(std::string_view path) { }

graphics::Mesh& assets::AssetDatabase::getOrLoadMesh(std::string_view path)
{
    const AssetId id = idFromPath(path);
    if (!meshes.contains(id))
    {
        loadMesh(path);
    }
    return getMesh(id);
}

assets::AssetId assets::AssetDatabase::createMesh(std::string_view name)
{
    using namespace graphics;

    AssetInfo info;
    info.path = fmt::format("{}/mesh/{}", RUNTIME_PATH, name);
    info.id = idFromPath(info.path);
    info.isRuntime = true;
    info.type = AssetInfo::AssetType::Mesh;

    assert(!metadata.contains(info.id));
    metadata.insert({info.id, info});



    return info.id;
}
