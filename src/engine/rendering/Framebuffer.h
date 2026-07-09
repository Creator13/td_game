#pragma once
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
        TextureFormat _textureFormat;
        bool _hasDepth;

        // GL handles
        gl::framebuffer_t _fbo;
        assets::AssetRef<Texture> _colorAttachment;
        gl::Uint _depthAttachment = 0; // Default value?

        bool _isCreated = false;

    public:
        Framebuffer(int width, int height, TextureFormat textureFormat, bool depth);
        ~Framebuffer();

        void create();
        void setSize(int width, int height);

    private:
        void createAttachments();
        void deleteAttachments();
        void validate();
    };
}
