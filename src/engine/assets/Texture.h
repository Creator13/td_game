#pragma once

#include <optional>
#include <vector>

#include "AssetId.h"

namespace core
{
    enum class TextureFormat
    {
        sRGBA32, ARGB32, RGB24, DXT1, DXT5, RGBASingle, RGBADouble, RGBAHalf
    };

    struct Texture
    {
        ~Texture();

        uint32_t getGlBindPoint() const { return _glBindPoint; };
        uint32_t getWidth() const { return _width; }
        uint32_t getHeight() const { return _height; }
        TextureFormat getFormat() const { return _format; }

        // TODO things like editing (setPixel, apply)

        static Texture create(uint32_t width, uint32_t height);
        static Texture loadFromFile(std::string_view path, bool readable = true);

    private:
        Texture() = default;

        uint32_t _glBindPoint = 0;

        uint32_t _width = 0, _height = 0;
        TextureFormat _format = TextureFormat::sRGBA32;

        bool _isReadable = false;
        std::optional<assets::AssetId> _uid;
        std::optional<std::vector<uint8_t>> _pixelData;

        void uploadPixelData() const;
        void uploadExternalData(const uint8_t* pixelData) const;
    };
}
