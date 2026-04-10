#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include "assets/AssetId.h"
#include "assets/AssetInfo.h"
#include "assets/AssetRef.h"
#include "assets/Mesh.h"
#include "assets/ShaderLoader.h"
#include "assets/Texture.h"
#include "rendering/Material.h"
#include "rendering/Pipeline.h"
#include "util/PagedStorage.h"

namespace core::assets
{
    class AssetDatabase
    {
        // Befriend every asset type that wants to construct assets in the database's storage.
        friend gfx::Pipeline;
        friend Mesh;
        friend Texture;

        static std::unique_ptr<AssetDatabase> instance;

        ShaderLoader _shaderLoader;
        util::PagedStorage<gfx::Pipeline, 64> _pipelineStorage;
        util::PagedStorage<Material, 64> _materialStorage;
        util::PagedStorage<Texture, 128> _textureStorage;
        util::PagedStorage<Mesh, 64> _meshStorage;

        std::filesystem::path _resourceRootPath;
        std::unordered_map<AssetId, AssetInfo> _registry;

        explicit AssetDatabase(std::string_view resourceRoot);

    public:
        static void initialize(std::string_view resourceRoot);
        static void destroy();

        static std::filesystem::path resolveResourcePath(std::string_view path);

        static const AssetInfo& getAssetInfo(AssetId assetId);
        template<C_AssetType T>
        static const AssetInfo& getAssetInfo(AssetRef<T> assetId);

        template<C_AssetType T>
        static AssetRef<T> tryGetAsset(AssetId assetId);

        static bool hasAsset(std::string_view assetName);
        static bool hasAsset(AssetId assetId);

        template<C_AssetType T>
        static AssetRef<T> registerAsset(std::string_view path, T* assetPtr);
        template<C_AssetType T>
        static AssetRef<T> registerRuntimeAsset(std::string_view name, T* assetPtr);
    };


    template<C_AssetType T>
    const AssetInfo& AssetDatabase::getAssetInfo(AssetRef<T> assetId)
    {
        return getAssetInfo(assetId.id());
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::tryGetAsset(AssetId assetId)
    {
        if (hasAsset(assetId))
        {
            return instance->_registry.at(assetId).getRef<T>();
        }
        return AssetRef<T>::null();
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::registerAsset(std::string_view path, T* assetPtr)
    {
        AssetId id = AssetId::idFromPath(path);
        ENGINE_ASSERT(!hasAsset(id), "Registering asset that already exists! Check for existence first before registering.");

        AssetInfo info = AssetInfo(path, id, AssetTraits<T>::type, false, assetPtr);
        instance->_registry.emplace(id, info);
        return AssetRef<T>(assetPtr, id);
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::registerRuntimeAsset(std::string_view name, T* assetPtr)
    {
        std::string runtimePath = fmt::format("@runtime/{}/{}",
            magic_enum::enum_name(AssetTraits<T>::type),
            name);

        AssetId id = AssetId::idFromPath(runtimePath);
        if (hasAsset(id))
        {
            int postfix = 1;
            do
            {
                postfix++;
                runtimePath = fmt::format("@runtime/{}/{}-{}",
                    magic_enum::enum_name(AssetTraits<T>::type),
                    name,
                    postfix);
                id = AssetId::idFromPath(runtimePath);
            } while (hasAsset(id));
        }
        ENGINE_ASSERT(!hasAsset(id), "Big fucking illegal? ?!");

        AssetInfo info = AssetInfo(std::move(runtimePath), id, AssetTraits<T>::type, true, assetPtr);
        instance->_registry.emplace(id, info);
        return AssetRef<T>(assetPtr, id);
    }
}
