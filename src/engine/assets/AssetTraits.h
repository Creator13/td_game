#pragma once

namespace core::assets
{
    enum class AssetType { Mesh, Texture, Pipeline, Material, Font };

    template<typename T>
    struct AssetTraits;

    template<typename T>
    concept C_AssetType = requires { AssetTraits<T>::type; };

}

namespace core
{
    namespace gfx
    {
        class Pipeline;
    }

    template<>
    struct assets::AssetTraits<gfx::Pipeline>
    {
        static constexpr AssetType type = AssetType::Pipeline;
    };

    class Material;

    template<>
    struct assets::AssetTraits<Material>
    {
        static constexpr AssetType type = AssetType::Material;
    };
}