#include "GraphicsBuffer.h"

#include <glad/gl.h>

#include "Logging.h"
#include "core/Assert.h"
#include "formatting/fmt_bytes.h"
#include "formatting/fmt_gl.h"

core::GraphicsBuffer::GraphicsBuffer(u32 size)
    : _size(size), _currentDataSize(0)
{
    glCreateBuffers(1, &_handle.id);
    glNamedBufferData(_handle.id, size, nullptr, GL_DYNAMIC_DRAW);

    _localBuffer.resize(size);
}

core::GraphicsBuffer::~GraphicsBuffer()
{
    if (_handle > 0)
    {
        glDeleteBuffers(1, &_handle.id);
    }
}

void core::GraphicsBuffer::clear()
{
    _currentDataSize = 0;
    _localBuffer.clear();
}

void core::GraphicsBuffer::upload()
{
    ZoneScopedN("GraphicsBuffer::uploadLocalBuffer");

    glNamedBufferData(_handle, _size, nullptr, GL_DYNAMIC_DRAW);
    glNamedBufferSubData(_handle, 0, _currentDataSize, _localBuffer.data());
}

void core::GraphicsBuffer::bind(gl::Int binding) const
{
    ENGINE_ASSERT(_handle != 0, "Trying to bind to uninitialized buffer");
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, _handle);
}

void core::GraphicsBuffer::resize(u32 newSize)
{
    ZoneScopedN("GraphicsBuffer::resize");

    if (_handle != 0)
    {
        glDeleteBuffers(1, &_handle.id);
    }

    glCreateBuffers(1, &_handle.id);
    glNamedBufferData(_handle.id, newSize, nullptr, GL_DYNAMIC_DRAW);
    _localBuffer.resize(newSize);
    _size = newSize;

    SPDLOG_DEBUG("Resizing graphics buffer (id:{}) to {:b}.", _handle, FormattableBytes{newSize});
}

