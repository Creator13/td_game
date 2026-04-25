#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <string_view>

#include "assets/AssetId.h"
#include "assets/AssetInfo.h"
#include "assets/AssetRef.h"
#include "assets/Font.h"
#include "assets/FontLoader.h"
#include "assets/Mesh.h"
#include "assets/ShaderLoader.h"
#include "assets/Texture.h"
#include "core/Assert.h"
#include "rendering/Material.h"
#include "rendering/Pipeline.h"
#include "util/PagedStorage.h"

namespace core::assets
{
    class AssetDatabase
    {
        // Befriend every asset type that wants to construct assets in the database's storage.
        // TODO Is this okay? Maybe revisit this some time...
        friend gfx::Pipeline;
        friend Mesh;
        friend Texture;
        friend Font;

        static std::unique_ptr<AssetDatabase> instance;

        // Loader systems with retained state
        // TODO move these systems towards built-time and load bytes directly into storage
        ShaderLoader _shaderLoader;
        FontLoader _fontLoader;

        util::PagedStorage<gfx::Pipeline, 64> _pipelineStorage;
        util::PagedStorage<Material, 64> _materialStorage;
        util::PagedStorage<Texture, 128> _textureStorage;
        util::PagedStorage<Mesh, 64> _meshStorage;
        util::PagedStorage<Font, 8> _fontStorage; // Fonts are huge and rare, keep page size small

        std::filesystem::path _resourceRootPath;
        std::unordered_map<AssetId, AssetInfo> _registry;

        explicit AssetDatabase(std::string_view resourceRoot);

        template<C_AssetType T>
        constexpr auto& getStorage();

    public:
        static void initialize(std::string_view resourceRoot);
        static void destroy();

        static std::filesystem::path resolveResourcePath(std::string_view path);

        static const AssetInfo& getAssetInfo(AssetId id);
        template<C_AssetType T>
        static const AssetInfo& getAssetInfo(AssetRef<T> assetRef);

        template<C_AssetType T>
        static AssetRef<T> tryGetAsset(AssetId id);

        static bool hasAsset(std::string_view assetName);
        static bool hasAsset(AssetId assetId);

        template<C_AssetType T>
        static AssetRef<T> registerAsset(std::string_view path, T* assetPtr, usize index);
        template<C_AssetType T>
        static AssetRef<T> registerRuntimeAsset(std::string_view name, T* assetPtr, usize index);

        template<C_AssetType T>
        static void deleteAsset(AssetRef<T> assetRef);
    };

    template<C_AssetType T>
    constexpr auto& AssetDatabase::getStorage()
    {
        if constexpr (std::is_same_v<T, gfx::Pipeline>) { return _pipelineStorage; }
        else if constexpr (std::is_same_v<T, Material>) { return _materialStorage; }
        else if constexpr (std::is_same_v<T, Texture>) { return _textureStorage; }
        else if constexpr (std::is_same_v<T, Mesh>) { return _meshStorage; }
        else if constexpr (std::is_same_v<T, Font>) { return _fontStorage; }
        else static_assert(false, "Asset type has no associated storage.");
    }

    template<C_AssetType T>
    const AssetInfo& AssetDatabase::getAssetInfo(AssetRef<T> assetRef)
    {
        return getAssetInfo(assetRef.id());
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::tryGetAsset(AssetId id)
    {
        ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

        if (hasAsset(id))
        {
            return instance->_registry.at(id).getRef<T>();
        }
        return AssetRef<T>::null();
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::registerAsset(std::string_view path, T* assetPtr, usize index)
    {
        ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

        AssetId id = AssetId::idFromPath(path);
        ENGINE_ASSERT(!hasAsset(id), "Registering asset that already exists! Check for existence first before registering.");

        AssetInfo info = AssetInfo(path, id, AssetTraits<T>::type, false, assetPtr, index);
        instance->_registry.emplace(id, info);
        return AssetRef<T>(assetPtr, id);
    }

    template<C_AssetType T>
    AssetRef<T> AssetDatabase::registerRuntimeAsset(std::string_view name, T* assetPtr, usize index)
    {
        // TODO since introducing index parameter, name is redundant? Maybe make it optional?
        ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");

        auto typeName = magic_enum::enum_name(AssetTraits<T>::type);

        std::string runtimePath = fmt::format("@runtime/{}/{}-{}",
            typeName, name, index);

        AssetId id = AssetId::idFromPath(runtimePath);
        ENGINE_ASSERT(!hasAsset(id), "A runtime asset with this name and index already exists! Name: {}, index: {}, type: {}",
            name, index, typeName);

        AssetInfo info = AssetInfo(std::move(runtimePath), id, AssetTraits<T>::type, true, assetPtr, index);
        instance->_registry.emplace(id, info);
        return AssetRef<T>(assetPtr, id);
    }

    template<C_AssetType T>
    void AssetDatabase::deleteAsset(AssetRef<T> assetRef)
    {
        ENGINE_ASSERT(instance != nullptr, "Usage of AssetDatabase before initialization.");
        ENGINE_ASSERT(hasAsset(assetRef.id()), "Tried to delete asset that does not exist (was it already deleted?)");

        const auto& info = instance->getAssetInfo(assetRef);
        instance->getStorage<T>().delete_at(info.containerIndex);
        instance->_registry.erase(info.id);
    }
}
