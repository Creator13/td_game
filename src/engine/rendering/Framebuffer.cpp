#include <glad/gl.h>

#include "Framebuffer.h"

#include "assets/AssetDatabase.h"

using namespace core;
using namespace core::gfx;
using namespace assets;

Framebuffer::Framebuffer(
    int width, int height,
    std::optional<TextureFormat> colorFormat,
    std::optional<TextureFormat> depthFormat, bool depthReadable)
    : _width(width), _height(height),
      _colorFormat(colorFormat),
      _depthFormat(depthFormat), _depthReadable(depthReadable)
{
    validateTextureFormats();
}

Framebuffer::~Framebuffer()
{
    deleteAttachments();

    // Delete fb object itself
    if (_fbo > 0)
    {
        glDeleteFramebuffers(1, &_fbo.id);
    }
}

void Framebuffer::create()
{
    ENGINE_ASSERT(_isCreated == false, "Framebuffer cannot be created more than once after each resize.");

    if (_fbo == 0)
    {
        glCreateFramebuffers(1, &_fbo.id);
    }

    deleteAttachments(); // Delete has no effect when called on null attachments, therefore we can call this function safely even on first create
    createAttachments();

    _isCreated = true;
    validate();
}

void Framebuffer::setSize(int width, int height)
{
    _width = width;
    _height = height;
    _isCreated = false;
}

void Framebuffer::createAttachments()
{
    if (_colorFormat.has_value())
    {
        _colorAttachment = Texture::create("anonymous framebuffer color texture",
            _width, _height, _colorFormat.value(), false, true,
            TextureWrap::Clamp, TextureWrap::Clamp, TextureFilter::Linear);

        glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0, _colorAttachment->getGlBindPoint(), 0);
    }

    if (_depthFormat.has_value())
    {
        if (_depthReadable)
        {
            AssetRef<Texture> depthTexture = Texture::create("anonymous framebuffer depth texture",
                _width, _height, _depthFormat.value(), false, true,
                TextureWrap::Clamp, TextureWrap::Clamp, TextureFilter::Linear);

            // Special case for the depth+stencil formats TODO: (extract into texture_util::isDepthStencilFormat function)
            if (_depthFormat.value() == TextureFormat::D24_UNORM_S8_UINT)
            {
                glNamedFramebufferTexture(_fbo, GL_DEPTH_STENCIL_ATTACHMENT, depthTexture->getGlBindPoint(), 0);
            }
            else
            {
                glNamedFramebufferTexture(_fbo, GL_DEPTH_ATTACHMENT, depthTexture->getGlBindPoint(), 0);
            }
            _depthAttachment = depthTexture;
        }
        else
        {
            // TODO abstract renderbuffer into a semantic gl::type alias, or even a RenderBuffer type ?
            gl::Uint bufferHandle;
            glCreateRenderbuffers(1, &bufferHandle);
            glNamedRenderbufferStorage(bufferHandle, texture_util::getGlInternalFormat(_depthFormat.value()), _width, _height);

            if (_depthFormat.value() == TextureFormat::D24_UNORM_S8_UINT)
            {
                glNamedFramebufferRenderbuffer(_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, bufferHandle);
            }
            else
            {
                glNamedFramebufferRenderbuffer(_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, bufferHandle);
            }
            _depthAttachment = bufferHandle;
        }

        // Special case: no color attachment
        if (!_colorFormat.has_value())
        {
            glNamedFramebufferDrawBuffer(_fbo, GL_NONE);
            glNamedFramebufferReadBuffer(_fbo, GL_NONE);
        }
    }
}

void Framebuffer::deleteAttachments()
{
    // Delete color
    if (_colorAttachment.isNotNull())
    {
        AssetDatabase::deleteAsset(_colorAttachment);
    }

    std::visit([]<typename T>(T& attachment)
    {
        if constexpr (std::is_same_v<T, std::monostate>) { /* no content, no deletion */ }
        else if constexpr (std::is_same_v<T, AssetRef<Texture>>)
        {
            if (attachment.isNotNull())
            {
                AssetDatabase::deleteAsset(attachment);
            }
        }
        else if constexpr (std::is_same_v<T, gl::Uint>)
        {
            if (attachment > 0)
            {
                glDeleteRenderbuffers(1, &attachment);
            }
        }
    }, _depthAttachment);

    _depthAttachment = std::monostate{ };
    _isCreated = false;
}

void Framebuffer::validate()
{
    ENGINE_ASSERT(_isCreated, "Framebuffer has not been created or create() has not been called after a resize.");

    gl::enum_t status = glCheckNamedFramebufferStatus(_fbo, GL_FRAMEBUFFER);
    if (status == GL_FRAMEBUFFER_COMPLETE) return;

    std::string_view errName;
    switch (status)
    {
        case GL_FRAMEBUFFER_UNDEFINED:
            errName = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            errName = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            ENGINE_UNREACHABLE();
    }

    // de-macro-ify to avoid nested macro (is this a necessary thing? it felt unsafe to nest)
    constexpr int completeStatusCode = GL_FRAMEBUFFER_COMPLETE;
    ENGINE_ASSERT(status == completeStatusCode, "Framebuffer (id {}) incomplete: {}", _fbo.id, errName);
}

bool Framebuffer::validateTextureFormats() const
{
    if (_colorFormat.has_value())
    {
        ENGINE_ASSERT(
            texture_util::isUncompressedColorFormat(_colorFormat.value()),
            "Invalid texture format for framebuffer color attachment: {}", magic_enum::enum_name(_colorFormat.value())
        );
    }
    if (_depthFormat.has_value())
    {
        ENGINE_ASSERT(
            texture_util::isDepthFormat(_depthFormat.value()),
            "Invalid texture format for framebuffer depth attachment: {}", magic_enum::enum_name(_depthFormat.value())
        );
    }
    return true;
}
