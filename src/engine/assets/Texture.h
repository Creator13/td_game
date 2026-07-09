#pragma once

#include <optional>
#include <vector>

#include "datatype.h"
#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"

namespace core
{
    struct Texture;

    template<>
    struct assets::AssetTraits<Texture>
    {
        static constexpr AssetType type = AssetType::Texture;
    };

    enum class TextureFormat : uint16_t
    {
        Unknown = 0,

        R8_UNORM,
        RG8_UNORM,
        RGBA8_UNORM,
        RGBA8_SRGB,

        R16_UNORM,
        RG16_UNORM,
        RGBA16_UNORM,

        R16_FLOAT,
        RG16_FLOAT,
        RGBA16_FLOAT,
        R32_FLOAT,
        RGBA32_FLOAT,

        R11G11B10_FLOAT,
        RGB10A2_UNORM,

        D16_UNORM,
        D24_UNORM_S8_UINT,
        D32_FLOAT,

        BC1_RGB_UNORM,
        BC1_RGB_SRGB,
        BC3_RGBA_UNORM,
        BC3_RGBA_SRGB,
        BC4_UNORM,
        BC5_UNORM,
        BC6H_UFLOAT,
        BC7_RGBA_UNORM,
        BC7_RGBA_SRGB,
        Count
    };

    enum class TextureWrap : u8
    {
        Repeat,
        Clamp,
    };

    enum class TextureFilter : u8
    {
        Linear,
        Nearest
    };

    struct Texture
    {
        ~Texture();

        gl::texture_t getGlBindPoint() const { return _glBindPoint; }

        u32 getWidth() const { return _width; }
        u32 getHeight() const { return _height; }
        TextureFormat getFormat() const { return _format; }

        u32 rawSizeBytes() const;

        // TODO things like editing (setPixel, apply)

        [[nodiscard]] static assets::AssetRef<Texture> create(std::string_view name, u32 width, u32 height, TextureFormat format, bool createMips, bool readOnly, TextureWrap wrapU = TextureWrap::Repeat, TextureWrap wrapV = TextureWrap::Repeat, TextureFilter filter = TextureFilter::Linear);
        [[nodiscard]] static assets::AssetRef<Texture> loadFromFile(std::string_view path, TextureFormat format, bool readable, bool createMips);
        [[nodiscard]] static assets::AssetRef<Texture> fallbackWhite();

        // TODO make this function much more safe
        void uploadExternalData(const u8* pixelData, gl::enum_t pixelFormat, gl::enum_t pixelType, bool genMipMaps) const;

    private:
        Texture() = default;

        Texture(u32 width, u32 height, TextureFormat format)
            : _width(width), _height(height), _format(format) { }

        gl::texture_t _glBindPoint = 0;

        u32 _width = 0, _height = 0;
        bool _genMipMaps = false;
        TextureFormat _format = TextureFormat::Unknown;

        bool _isReadable = false;
        std::optional<std::vector<u8>> _pixelData;

        void uploadPixelData() const;

        static assets::AssetRef<Texture> _fallbackWhiteRef;
    };

    namespace texture_util
    {
        gl::enum_t getGlInternalFormat(TextureFormat format);
        u8 getChannelCountInFormat(TextureFormat format);
        gl::enum_t getGlPixelDataType(TextureFormat format);
        gl::enum_t getGlPixelFormat(TextureFormat format);

        gl::enum_t getGlTexWrap(TextureWrap wrap);
        gl::enum_t getGlTexFilter(TextureFilter filter, bool withMips);

        gl::texture_t createGlTexture(int width, int height, gl::enum_t internalFormat,
            bool createMips,
            TextureWrap wrapU = TextureWrap::Repeat, TextureWrap wrapV = TextureWrap::Repeat,
            TextureFilter filter = TextureFilter::Linear);
    }
}
