#pragma once

#include <array>
#include <string_view>

#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "assets/FontMetrics.h"
#include "assets/Texture.h"
#include "rendering/GraphicsBuffer.h"

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

        static assets::AssetRef<gfx::Pipeline> _defaultFontPipeline;

        assets::AssetRef<Texture> _fontTexture;
        assets::AssetRef<Material> _fontMaterial;
        assets::FontMetrics _fontMetrics;
        std::array<assets::GlyphMetrics, 256> _glyphMetrics { };
        GraphicsBuffer _glyphDataRenderBuffer;

        Font();

        static assets::AssetRef<gfx::Pipeline> getOrCreateDefaultPipeline();

    public:
        ~Font();

        assets::AssetRef<Material> getMaterial() const noexcept { return _fontMaterial; }
        GraphicsBuffer& getGlyphDataBuffer() noexcept { return _glyphDataRenderBuffer; }

        const assets::GlyphMetrics& getGlyphMetrics(char32_t codepoint) const;
        const assets::FontMetrics& getFontMetrics() const noexcept { return _fontMetrics; }

        [[nodiscard]] static assets::AssetRef<Font> loadFromFile(std::string_view path, assets::AssetRef<gfx::Pipeline> fontPipeline = assets::AssetRef<gfx::Pipeline>::null());
    };
}
