#pragma once

namespace core::assets
{
    enum class AssetType { Mesh, Shader, Texture };

    template<typename T>
    struct AssetTraits;

    template<typename T>
    concept C_AssetType = requires { AssetTraits<T>::type; };
}