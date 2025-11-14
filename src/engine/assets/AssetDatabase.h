#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

namespace graphics::shader
{
    struct ShaderProgramData;
}

namespace graphics
{
    class Mesh;
}

namespace assets
{
    typedef uint64_t AssetId;

    struct AssetInfo
    {
        enum class AssetType { Mesh, Shader };

        std::string path;
        AssetId id;
        bool isRuntime;
        AssetType type;
    };

    class AssetDatabase
    {
    public:
        explicit AssetDatabase(std::string_view resourceRoot = "res");
        ~AssetDatabase();

        static AssetId idFromPath(std::string_view path);

        AssetInfo getAssetInfo(AssetId id);

        graphics::Mesh& getMesh(AssetId id);
        void loadMesh(std::string_view path);
        graphics::Mesh& getOrLoadMesh(std::string_view path);
        AssetId createMesh(std::string_view name);

    private:
        std::unordered_map<AssetId, AssetInfo> metadata;

        std::unordered_map<AssetId, graphics::Mesh> meshes;
        std::unordered_map<AssetId, graphics::shader::ShaderProgramData> shaders;

        AssetId createMeshInteral(std::string_view name);
    };
}
