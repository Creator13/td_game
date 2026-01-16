#include "assets/Texture.h"

#include <stb_image.h>
#include <glad/glad.h>

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "core/Assert.h"
#include "rendering/Color.h"

using namespace core;

namespace
{
    GLuint createGlTexture(int width, int height)
    {
        GLuint texName;
        glCreateTextures(GL_TEXTURE_2D, 1, &texName);

        glTextureParameteri(texName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texName, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texName, GL_TEXTURE_WRAP_T, GL_REPEAT);

        const int mipLevels = static_cast<int>(math::floor(math::log2(math::max(width, height)))) + 1;

        glTextureStorage2D(texName, mipLevels, GL_SRGB8_ALPHA8, width, height);
        return texName;
    }
}

Texture::~Texture()
{
    // TODO clean up
}

void Texture::uploadPixelData() const
{
    ENGINE_ASSERT(_isReadable, "Cannot upload pixel data when data is not available in CPU memory (texture is not marked readable).");

    glTextureSubImage2D(_glBindPoint, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, _pixelData->data());
    glGenerateTextureMipmap(_glBindPoint);
}

void Texture::uploadExternalData(const uint8_t* pixelData) const
{
    glTextureSubImage2D(_glBindPoint, 0, 0, 0, _width, _height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glGenerateTextureMipmap(_glBindPoint);
}

Texture Texture::create(uint32_t width, uint32_t height)
{
    ENGINE_ASSERT(width > 0 && height > 0, "Width and height values should be greater than zero");

    Texture outTexture;
    outTexture._width = width;
    outTexture._height = height;
    outTexture._format = TextureFormat::sRGBA32;
    outTexture._isReadable = true;
    outTexture._pixelData = std::vector<uint8_t>(width * height * 4);
    outTexture._glBindPoint = createGlTexture(width, height);
    return outTexture;
}

Texture Texture::loadFromFile(std::string_view path, bool readable)
{
    const auto fullPath = assets::resolveResourcePath(path);
    std::optional<std::vector<uint8_t>> fileData = file::readFileBinary(fullPath);
    if (!fileData)
    {
        // TODO something like an invalid texture fallback? white?
        ENGINE_ASSERT(false, "Could not read texture file at {} ({}).", path, fullPath.string());
    }

    int width, height, channelsInFile;
    unsigned char* decodedPixelData = stbi_load_from_memory(
        fileData.value().data(),
        static_cast<int>(fileData.value().size()),
        &width, &height, &channelsInFile,
        4); // TODO forced 4 channels, this should be configurable with TextureFormat (but that opens a whole can of worms on conversions)

    Texture outTexture;
    outTexture._width = width;
    outTexture._height = height;
    outTexture._format = TextureFormat::sRGBA32;
    outTexture._isReadable = readable;

    outTexture._glBindPoint = createGlTexture(width, height);
    if (!readable)
    {
        outTexture._pixelData = std::nullopt;
        outTexture.uploadExternalData(decodedPixelData);
    }
    else
    {
        outTexture._pixelData = std::vector(decodedPixelData, decodedPixelData + width * height * 4);
        outTexture.uploadPixelData();
    }

    stbi_image_free(decodedPixelData);

    // TODO register this as an asset with the assetDatabase as soon as I implement asset registering.
    //  And that also allows dynamic loading of data into CPU memory.

    return outTexture;
}
