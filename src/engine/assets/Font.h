#pragma once

#include <array>
#include <string_view>

#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "assets/FontMetrics.h"
#include "assets/Texture.h"

namespace msdf_atlas {
    class Charset;
}

namespace core
{
    namespace assets {
        class FontLoader;
    }

    class Font;

    template<>
    struct assets::AssetTraits<Font>
    {
        static constexpr AssetType type = AssetType::Font;
    };

    class Font
    {
        friend assets::FontLoader;

        Font();

        assets::AssetRef<Texture> _fontTexture;
        assets::AssetRef<Material> _fontMaterial;
        assets::FontMetrics _fontMetrics;
        std::array<assets::GlyphMetrics, 256> _glyphMetrics { };

    public:
        ~Font();

        assets::AssetRef<Material> getMaterial() const noexcept { return _fontMaterial; }
        const assets::GlyphMetrics& getGlyphMetrics(u32 codepoint) const;
        const assets::FontMetrics& getFontMetrics() const noexcept { return _fontMetrics; }

        [[nodiscard]] static assets::AssetRef<Font> loadFromFile(std::string_view path, assets::AssetRef<gfx::Pipeline> fontPipeline);
    };
}
