#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>

#include "rendering/Mesh.h"
#include "rendering/shader.h"

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

    private:
        struct MeshGpuAllocator
        {
            std::unordered_map<AssetId, graphics::MeshGpuHandle> handles;

            const graphics::MeshGpuHandle& get(AssetId id) const;
            void upload(AssetId id, const graphics::Mesh& mesh);
            void clean();
        };

        std::string resourceRoot;

        std::unordered_map<AssetId, AssetInfo> metadata;

        std::unordered_map<AssetId, graphics::Mesh> meshes;
        MeshGpuAllocator meshAllocator;
        std::unordered_map<AssetId, graphics::shader::ShaderProgramData> shaders;

        static std::string makeInternalPath(AssetInfo::AssetType type, std::string_view name);

        void loadInternalMesh(std::string_view name, const graphics::Vertex* vPtr, uint32_t vCnt, const uint32_t* iPtr, uint32_t iCnt);
        void loadInternalMeshes();
        AssetId createMesh(std::string_view name);
    };
}
