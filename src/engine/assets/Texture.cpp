#include "assets/Texture.h"

#include <stb_image.h>
#include <glad/gl.h>

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
    gl::texture_t createGlTexture(int width, int height, gl::enum_t internalFormat)
    {
        gl::texture_t texName;
        glCreateTextures(GL_TEXTURE_2D, 1, &texName.id);

        glTextureParameteri(texName, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texName, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(texName, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texName, GL_TEXTURE_WRAP_T, GL_REPEAT);

        const int mipLevels = static_cast<int>(math::floor(math::log2(math::max(width, height)))) + 1;

        glTextureStorage2D(texName, mipLevels, internalFormat, width, height);
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
    uploadExternalData(_pixelData->data(), texture_util::getGlPixelFormat(_format), texture_util::getGlPixelDataType(_format));
}

void Texture::uploadExternalData(const uint8_t* pixelData, gl::enum_t pixelFormat, gl::enum_t pixelType) const
{
    glTextureSubImage2D(_glBindPoint, 0, 0, 0, _width, _height, pixelFormat, pixelType, pixelData);
    glGenerateTextureMipmap(_glBindPoint);
}

AssetRef<Texture> Texture::create(uint32_t width, uint32_t height, TextureFormat format)
{
    ENGINE_ASSERT(width > 0 && height > 0, "Width and height values should be greater than zero");

    Texture* outTexture = textureStorage.allocate_uninitialized();
    ::new(outTexture) Texture(width, height, format);

    outTexture->_isReadable = true;
    outTexture->_pixelData = std::vector<u8>(width * height * 4);
    outTexture->_glBindPoint = createGlTexture(width, height, texture_util::getGlInternalFormat(format));
    outTexture->_genMipMaps = false;
    return AssetRef<Texture>::null();
}

AssetRef<Texture> Texture::loadFromFile(std::string_view path, TextureFormat format, bool readable)
{
    // I'm not entirely sure why I *don't* need to flip images vertically, but I'm pretty sure it's the coordinate
    //  system conversion I do from opengl standard Y+ up to my Z+ up.
    // stbi_set_flip_vertically_on_load(true);

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
        texture_util::getFormatChannelCount(format)); // TODO forced 4 channels, this should be configurable with TextureFormat (but that opens a whole can of worms on conversions)

    Texture* outTexture = textureStorage.allocate_uninitialized();
    ::new(outTexture) Texture(width, height, format);

    outTexture->_isReadable = readable;
    outTexture->_glBindPoint = createGlTexture(width, height, texture_util::getGlInternalFormat(format));

    if (!readable)
    {
        outTexture->_pixelData = std::nullopt;
        outTexture->uploadExternalData(decodedPixelData, texture_util::getGlPixelFormat(format), texture_util::getGlPixelDataType(format));
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

constexpr gl::enum_t texture_util::getGlInternalFormat(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8_UNORM:
            return GL_R8;
        case TextureFormat::RG8_UNORM:
            return GL_RG8;
        case TextureFormat::RGBA8_UNORM:
            return GL_RGBA8;
        case TextureFormat::RGBA8_SRGB:
            return GL_SRGB8_ALPHA8;

        case TextureFormat::R16_UNORM:
            return GL_R16;
        case TextureFormat::RG16_UNORM:
            return GL_RG16;
        case TextureFormat::RGBA16_UNORM:
            return GL_RGBA16;

        case TextureFormat::R16_FLOAT:
            return GL_R16F;
        case TextureFormat::RG16_FLOAT:
            return GL_RG16F;
        case TextureFormat::RGBA16_FLOAT:
            return GL_RGBA16F;
        case TextureFormat::R32_FLOAT:
            return GL_R32F;
        case TextureFormat::RGBA32_FLOAT:
            return GL_RGBA32F;

        case TextureFormat::R11G11B10_FLOAT:
            return GL_R11F_G11F_B10F;
        case TextureFormat::RGB10A2_UNORM:
            return GL_RGB10_A2;

        case TextureFormat::D16_UNORM:
            return GL_DEPTH_COMPONENT16;
        case TextureFormat::D24_UNORM_S8_UINT:
            return GL_DEPTH24_STENCIL8;
        case TextureFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT32F;

        case TextureFormat::BC1_RGB_UNORM:
            return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        case TextureFormat::BC1_RGB_SRGB:
            return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        case TextureFormat::BC3_RGBA_UNORM:
            return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        case TextureFormat::BC3_RGBA_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
        case TextureFormat::BC4_UNORM:
            return GL_COMPRESSED_RED_RGTC1;
        case TextureFormat::BC5_UNORM:
            return GL_COMPRESSED_RG_RGTC2;
        case TextureFormat::BC6H_UFLOAT:
            return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
        case TextureFormat::BC7_RGBA_UNORM:
            return GL_COMPRESSED_RGBA_BPTC_UNORM;
        case TextureFormat::BC7_RGBA_SRGB:
            return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;

        case TextureFormat::Count:
        case TextureFormat::Unknown:
        default:
            ENGINE_ASSERT(false, "Unknown texture format");
            return 0; // Unreachable post-assert
    }
}

constexpr u8 texture_util::getFormatChannelCount(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8_UNORM:
        case TextureFormat::R16_UNORM:
        case TextureFormat::R16_FLOAT:
        case TextureFormat::R32_FLOAT:
        case TextureFormat::BC4_UNORM:
            return 1;

        case TextureFormat::RG8_UNORM:
        case TextureFormat::RG16_UNORM:
        case TextureFormat::RG16_FLOAT:
        case TextureFormat::BC5_UNORM:
            return 2;

        case TextureFormat::R11G11B10_FLOAT:
        case TextureFormat::BC1_RGB_UNORM:
        case TextureFormat::BC1_RGB_SRGB:
        case TextureFormat::BC6H_UFLOAT:
            return 3;

        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA16_UNORM:
        case TextureFormat::RGBA16_FLOAT:
        case TextureFormat::RGBA32_FLOAT:
        case TextureFormat::RGB10A2_UNORM:
        case TextureFormat::BC3_RGBA_UNORM:
        case TextureFormat::BC3_RGBA_SRGB:
        case TextureFormat::BC7_RGBA_UNORM:
        case TextureFormat::BC7_RGBA_SRGB:
            return 4;

        default:
            return 0;
    }
}

constexpr gl::enum_t texture_util::getGlPixelDataType(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8_UNORM:
        case TextureFormat::RG8_UNORM:
        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA8_SRGB:
            return GL_UNSIGNED_BYTE;

        case TextureFormat::R16_UNORM:
        case TextureFormat::RG16_UNORM:
        case TextureFormat::RGBA16_UNORM:
        case TextureFormat::D16_UNORM:
            return GL_UNSIGNED_SHORT;

        case TextureFormat::R11G11B10_FLOAT:
            return GL_UNSIGNED_INT_10F_11F_11F_REV;
        case TextureFormat::RGB10A2_UNORM:
            return GL_UNSIGNED_INT_10_10_10_2;

        case TextureFormat::R16_FLOAT:
        case TextureFormat::RG16_FLOAT:
        case TextureFormat::RGBA16_FLOAT:
            return GL_HALF_FLOAT;

        case TextureFormat::R32_FLOAT:
        case TextureFormat::RGBA32_FLOAT:
        case TextureFormat::D32_FLOAT:
            return GL_FLOAT;

        case TextureFormat::D24_UNORM_S8_UINT:
            return GL_UNSIGNED_INT_24_8;

        case TextureFormat::BC1_RGB_UNORM:
        case TextureFormat::BC1_RGB_SRGB:
        case TextureFormat::BC3_RGBA_UNORM:
        case TextureFormat::BC3_RGBA_SRGB:
        case TextureFormat::BC4_UNORM:
        case TextureFormat::BC5_UNORM:
        case TextureFormat::BC6H_UFLOAT:
        case TextureFormat::BC7_RGBA_UNORM:
        case TextureFormat::BC7_RGBA_SRGB:
            ENGINE_ASSERT(false, "Illegal call to get GL pixel data type for a compressed texture format (should not be interpreting compressed data as uncompressed pixels)");
            return 0; // Unreachable post-assert

        case TextureFormat::Count:
        case TextureFormat::Unknown:
        default:
            ENGINE_ASSERT(false, "Unknown texture format");
            return 0; // Unreachable post-assert
    }
}

constexpr gl::enum_t texture_util::getGlPixelFormat(TextureFormat format)
{
    switch (format)
    {
        case TextureFormat::R8_UNORM:
        case TextureFormat::R16_UNORM:
        case TextureFormat::R16_FLOAT:
        case TextureFormat::R32_FLOAT:
            return GL_RED;

        case TextureFormat::RG8_UNORM:
        case TextureFormat::RG16_UNORM:
        case TextureFormat::RG16_FLOAT:
            return GL_RG;

        case TextureFormat::R11G11B10_FLOAT:
            return GL_RGB;

        case TextureFormat::RGBA8_UNORM:
        case TextureFormat::RGBA8_SRGB:
        case TextureFormat::RGBA16_UNORM:
        case TextureFormat::RGBA16_FLOAT:
        case TextureFormat::RGBA32_FLOAT:
        case TextureFormat::RGB10A2_UNORM:
            return GL_RGBA;

        case TextureFormat::D16_UNORM:
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_FLOAT:
            return GL_DEPTH_COMPONENT;

        case TextureFormat::BC1_RGB_UNORM:
        case TextureFormat::BC1_RGB_SRGB:
        case TextureFormat::BC3_RGBA_UNORM:
        case TextureFormat::BC3_RGBA_SRGB:
        case TextureFormat::BC4_UNORM:
        case TextureFormat::BC5_UNORM:
        case TextureFormat::BC6H_UFLOAT:
        case TextureFormat::BC7_RGBA_UNORM:
        case TextureFormat::BC7_RGBA_SRGB:
            ENGINE_ASSERT(false, "Illegal call to get GL pixel format for a compressed texture format (should not be interpreting compressed data as uncompressed pixels)");
            return 0; // Unreachable post-assert

        case TextureFormat::Count:
        case TextureFormat::Unknown:
        default:
            ENGINE_ASSERT(false, "Unknown texture format.");
            return 0; // Unreachable post-assert
    }
}
