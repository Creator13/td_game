#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "assets/AssetId.h"
#include "rendering/Mesh.h"
#include "rendering/shader.h"

namespace fastgltf
{
    class Parser;
}

namespace assets
{
    struct AssetInfo
    {
        enum class AssetType { Mesh, Shader, Texture };

        std::string path;
        AssetId id;
        bool isRuntime;
        AssetType type;
    };

    class AssetDatabase
    {
    public:
        AssetDatabase() = delete;
        explicit AssetDatabase(std::string_view resourceRoot = "res");
        ~AssetDatabase();

        static const AssetInfo& getAssetInfo(AssetId id);

        static const graphics::MeshGpuHandle& getMeshGpuHandle(AssetId id);
        static const graphics::Mesh& getMeshView(AssetId id);
        static graphics::Mesh& getMeshMut(AssetId id);
        static AssetId loadMeshFromFile(std::string_view path);
        static AssetId createRuntimeMesh(std::string_view name);
        static AssetId loadTextureFromFile(std::string_view path);

        static const graphics::ShaderProgramData& getShaderProgram(AssetId id);
        static AssetId loadShaderFromFiles(std::string_view vertPath, std::string_view fragPath);

        std::filesystem::path resolveResourcePath(std::string_view path) const;

    private:
        struct MeshGpuAllocator
        {
            MeshGpuAllocator() : handles(8) { }
            explicit MeshGpuAllocator(int capacity) : handles(capacity) { }

            std::unordered_map<AssetId, graphics::MeshGpuHandle> handles;

            const graphics::MeshGpuHandle& get(AssetId id) const;
            void upload(AssetId id, const graphics::Mesh& mesh);
            void clean();
        };

        std::unique_ptr<fastgltf::Parser> gltfParser;

        std::filesystem::path rootPath;

        std::unordered_map<AssetId, AssetInfo> metadata;

        std::unordered_map<AssetId, graphics::Mesh> meshes;
        MeshGpuAllocator meshAllocator;

        std::unordered_map<AssetId, uint32_t> shaderStageCache;
        std::unordered_map<AssetId, graphics::ShaderProgramData> shaders;

        std::unordered_map<AssetId, uint32_t> textures;

        static std::string makeInternalPath(AssetInfo::AssetType type, std::string_view name);

        void loadInternalMesh(std::string_view name, const graphics::Vertex* vPtr, uint32_t vCnt, const uint32_t* iPtr, uint32_t iCnt, const math::AABB& bounds);
        void loadInternalMeshes();
        AssetId createMesh(std::string_view name);

        void loadInternalShaders();
        std::optional<uint32_t> loadShaderStageFromFile(std::string_view path, uint32_t stageType);
    };

    void bindAssetDatabase(AssetDatabase& db);
    std::filesystem::path resolveResourcePath(std::string_view path);
}
