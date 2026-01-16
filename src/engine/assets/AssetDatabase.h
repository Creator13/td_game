#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "assets/AssetId.h"
#include "rendering/Mesh.h"
#include "rendering/shader.h"
#include "util/PagedStorage.h"

namespace core {
    struct Texture;
}

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

        static const core::MeshGpuHandle& getMeshGpuHandle(AssetId id);
        static const core::Mesh& getMeshView(AssetId id);
        static core::Mesh& getMeshMut(AssetId id);
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

            std::unordered_map<AssetId, core::MeshGpuHandle> handles;

            const core::MeshGpuHandle& get(AssetId id) const;
            void upload(AssetId id, const core::Mesh& mesh);
            void clean();
        };

        std::unique_ptr<fastgltf::Parser> gltfParser;

        std::filesystem::path rootPath;

        std::unordered_map<AssetId, AssetInfo> metadata;

        std::unordered_map<AssetId, core::Mesh> meshes;
        MeshGpuAllocator meshAllocator;

        std::unordered_map<AssetId, uint32_t> shaderStageCache;
        std::unordered_map<AssetId, graphics::ShaderProgramData> shaders;

        std::unordered_map<AssetId, uint32_t> textures;

        static std::string makeInternalPath(AssetInfo::AssetType type, std::string_view name);

        void loadInternalMesh(std::string_view name, const core::Vertex* vPtr, uint32_t vCnt, const uint32_t* iPtr, uint32_t iCnt, const math::AABB& bounds);
        void loadInternalMeshes();
        AssetId createMesh(std::string_view name);

        void loadInternalShaders();
        std::optional<uint32_t> loadShaderStageFromFile(std::string_view path, uint32_t stageType);
    };

    struct AssetStorage
    {
        static constexpr size_t PAGE_SIZE = 128;
        util::PagedStorage<core::Mesh, PAGE_SIZE> meshes;
        util::PagedStorage<core::Texture, PAGE_SIZE> textures;
        util::PagedStorage<graphics::ShaderProgramData, PAGE_SIZE> shaders;
    };

    void bindAssetDatabase(AssetDatabase& db);

    void bindAssetStorage(AssetStorage& storage);
    std::filesystem::path resolveResourcePath(std::string_view path);

    void registerShaderProgram(const graphics::ShaderProgramData& shader);
    void registerMesh(const core::Mesh& mesh);
    void registerTexture(const core::Texture& texture);
}
