#include "assets/Texture.h"

#include <stb_image.h>
#include <glad/glad.h>

#include "assets/AssetDatabase.h"
#include "assets/File.h"
#include "core/Assert.h"
#include "math/func.h"
#include "util/PagedStorage.h"

using namespace core;
using namespace assets;

namespace
{
    util::PagedStorage<Texture, 128> textureStorage;
}

namespace
{
    gl::texture_t createGlTexture(int width, int height)
    {
        gl::texture_t texName;
        glCreateTextures(GL_TEXTURE_2D, 1, &texName.id);

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

assets::AssetRef<Texture> Texture::create(uint32_t width, uint32_t height)
{
    ENGINE_ASSERT(width > 0 && height > 0, "Width and height values should be greater than zero");

    Texture* outTexture = textureStorage.allocate_uninitialized();
    ::new(outTexture) Texture(width, height, TextureFormat::sRGBA32);

    outTexture->_isReadable = true;
    outTexture->_pixelData = std::vector<u8>(width * height * 4);
    outTexture->_glBindPoint = createGlTexture(width, height);
    return AssetRef<Texture>::null();
}

AssetRef<Texture> Texture::loadFromFile(std::string_view path, bool readable)
{
    const auto fullPath = resolveResourcePath(path);
    std::optional<std::vector<u8>> fileData = file::readFileBinary(fullPath);
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

    Texture* outTexture = textureStorage.allocate_uninitialized();
    ::new(outTexture) Texture(width, height, TextureFormat::sRGBA32);

    outTexture->_isReadable = readable;
    outTexture->_glBindPoint = createGlTexture(width, height);

    if (!readable)
    {
        outTexture->_pixelData = std::nullopt;
        outTexture->uploadExternalData(decodedPixelData);
    }
    else
    {
        outTexture->_pixelData = std::vector(decodedPixelData, decodedPixelData + width * height * 4);
        outTexture->uploadPixelData();
    }

    stbi_image_free(decodedPixelData);

    const AssetId id = registerAsset(path, AssetType::Texture, outTexture);
    return AssetRef(outTexture, id);
}
