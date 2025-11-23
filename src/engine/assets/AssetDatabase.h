#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "rendering/Mesh.h"
#include "rendering/shader.h"

namespace fastgltf
{
    class Parser;
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
        AssetDatabase() = delete;
        explicit AssetDatabase(std::string_view resourceRoot = "res");
        ~AssetDatabase();

        static AssetId idFromPath(std::string_view path);

        const AssetInfo& getAssetInfo(AssetId id);

        const graphics::MeshGpuHandle& getMeshGpuHandle(AssetId id) const;
        const graphics::Mesh& getMeshView(AssetId id) const;
        graphics::Mesh& getMeshMut(AssetId id);
        AssetId loadMeshFromFile(std::string_view path);
        AssetId createRuntimeMesh(std::string_view name);

        const graphics::ShaderProgramData& getShaderProgram(AssetId id) const;
        AssetId loadShaderFromFiles(std::string_view vertPath, std::string_view fragPath);

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

        static std::string makeInternalPath(AssetInfo::AssetType type, std::string_view name);

        std::string resolveResourcePath(std::string_view path) const;

        void loadInternalMesh(std::string_view name, const graphics::Vertex* vPtr, uint32_t vCnt, const uint32_t* iPtr, uint32_t iCnt);
        void loadInternalMeshes();
        AssetId createMesh(std::string_view name);

        void loadInternalShaders();
        std::optional<uint32_t> loadShaderStageFromFile(std::string_view path, uint32_t stageType);
    };
}
