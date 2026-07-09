#include <glad/gl.h>

#include "Framebuffer.h"

using namespace core;
using namespace core::gfx;

Framebuffer::Framebuffer(int width, int height, TextureFormat textureFormat, bool depth)
    : _width(width), _height(height), _textureFormat(textureFormat), _hasDepth(depth) { }

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

    deleteAttachments(); // Delete has no effect when called on null attachments, hence we can call this function safely even on first create
    createAttachments();

    _isCreated = true;
    validate();
}

void Framebuffer::createAttachments()
{
    // TODO validate texture format to be an allowed format for framebuffers (not every format is allowed, see compressed formats)

    _colorAttachment = Texture::create("anonymous framebuffer texture",
        _width, _height, _textureFormat, false, true,
        TextureWrap::Clamp, TextureWrap::Clamp, TextureFilter::Linear);

    glNamedFramebufferTexture(_fbo, GL_COLOR_ATTACHMENT0, _colorAttachment->getGlBindPoint(), 0);

    if (_hasDepth)
    {
        // TODO allow use of texture instead of renderbuffer, AND abstract renderbuffer into a gl::type
        glCreateRenderbuffers(1, &_depthAttachment);
        glNamedRenderbufferStorage(_depthAttachment, GL_DEPTH24_STENCIL8, _width, _height);

        glNamedFramebufferRenderbuffer(_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthAttachment);
    }
}

void Framebuffer::deleteAttachments()
{
    // TODO _colorattachment is an asset, and assets cannot be deleted atm so fix that (PPLEASE allow textures to not be assets?? or something??)
    // if (_colorAttachment > 0)
    // {
    //     glDeleteTextures(1, &_colorAttachment.id);
    // }
    if (_depthAttachment > 0)
    {
        glDeleteRenderbuffers(1, &_depthAttachment);
    }

    _isCreated = false;
}

void Framebuffer::setSize(int width, int height)
{
    _width = width;
    _height = height;
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
