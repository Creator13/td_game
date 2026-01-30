#pragma once

#include <optional>
#include <vector>

#include "assets/AssetRef.h"
#include "assets/AssetTraits.h"
#include "datatype.h"


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

    struct Texture
    {
        ~Texture();

        gl::texture_t getGlBindPoint() const { return _glBindPoint; }

        u32 getWidth() const { return _width; }
        u32 getHeight() const { return _height; }
        TextureFormat getFormat() const { return _format; }

        // TODO things like editing (setPixel, apply)

        static assets::AssetRef<Texture> create(u32 width, u32 height, TextureFormat format);
        static assets::AssetRef<Texture> loadFromFile(std::string_view path, TextureFormat format, bool readable = true);

    private:
        Texture() = default;

        Texture(u32 width, u32 height, TextureFormat format)
            : _width(width), _height(height), _format(format) { }

        gl::texture_t _glBindPoint = 0;

        u32 _width = 0, _height = 0;
        bool _genMipMaps = false;
        TextureFormat _format = TextureFormat::Unknown;

        bool _isReadable = false;
        std::optional<assets::AssetId> _uid;
        std::optional<std::vector<u8>> _pixelData;

        void uploadPixelData() const;
        void uploadExternalData(const uint8_t* pixelData, gl::enum_t pixelFormat, gl::enum_t pixelType) const;
    };

    namespace texture_util
    {
        constexpr gl::enum_t getGlInternalFormat(TextureFormat format);
        constexpr u8 getFormatChannelCount(TextureFormat format);
        constexpr gl::enum_t getGlPixelDataType(TextureFormat format);
        constexpr gl::enum_t getGlPixelFormat(TextureFormat format);
    }
}
