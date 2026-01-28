#pragma once

#include "assets/AssetRef.h"
#include "assets/Shader.h"
#include "assets/Texture.h"
#include "rendering/Color.h"

namespace core
{
    struct Material
    {
        assets::AssetRef<Shader> shader;

        assets::AssetRef<Texture> albedo;
        graphics::ColorLinear baseColor;
    };
}
