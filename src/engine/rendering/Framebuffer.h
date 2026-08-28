#pragma once

#include <optional>
#include <variant>

#include "datatype.h"
#include "assets/Texture.h"

namespace core::gfx
{
    class Renderer;

    class Framebuffer
    {
        friend Renderer;

    private:
        // Properties
        int _width, _height;
        std::optional<TextureFormat> _colorFormat;
        std::optional<TextureFormat> _depthFormat;
        bool _depthReadable;

        // GL handles
        gl::framebuffer_t _fbo;
        assets::AssetRef<Texture> _colorAttachment;
        std::variant<std::monostate, assets::AssetRef<Texture>, gl::Uint> _depthAttachment; // This is a variant between a database-managed texture and a self-managed renderbuffer handle (and it's ugly, but there is not currently a better solution (TODO allow non-asset texture handles))

        bool _isCreated = false;

    public:
        Framebuffer(
            int width, int height,
            std::optional<TextureFormat> colorFormat,
            std::optional<TextureFormat> depthFormat, bool depthReadable);
        ~Framebuffer();

        void create();
        void setSize(int width, int height);

    private:
        void createAttachments();
        void deleteAttachments();

        void validate();
        bool validateTextureFormats() const;
    };
}
