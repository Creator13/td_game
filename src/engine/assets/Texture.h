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

    enum class TextureFormat
    {
        sRGBA32, ARGB32, RGB24, DXT1, DXT5, RGBASingle, RGBADouble, RGBAHalf
    };

    struct Texture
    {
        ~Texture();

        gl::texture_t getGlBindPoint() const { return _glBindPoint; }

        u32 getWidth() const { return _width; }
        u32 getHeight() const { return _height; }
        TextureFormat getFormat() const { return _format; }

        // TODO things like editing (setPixel, apply)

        static assets::AssetRef<Texture> create(u32 width, u32 height);
        static assets::AssetRef<Texture> loadFromFile(std::string_view path, bool readable = true);

    private:
        Texture() = default;

        Texture(u32 width, u32 height, TextureFormat format = TextureFormat::sRGBA32)
            : _width(width), _height(height), _format(format) { }

        gl::texture_t _glBindPoint = 0;

        u32 _width = 0, _height = 0;
        TextureFormat _format = TextureFormat::sRGBA32;

        bool _isReadable = false;
        std::optional<assets::AssetId> _uid;
        std::optional<std::vector<u8>> _pixelData;

        void uploadPixelData() const;
        void uploadExternalData(const uint8_t* pixelData) const;
    };
}
