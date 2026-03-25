#include "GrowableUbo.h"

#include <glad/gl.h>

#include "Logging.h"
#include "core/Assert.h"
#include "formatting/fmt_bytes.h"
#include "formatting/fmt_gl.h"

using namespace core::gfx;

GrowableUbo::GrowableUbo(u32 size)
    : _size(size), _stride(1), _elemSize(1)
{
    if (_glUboOffsetAlignment == 0)
    {
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &_glUboOffsetAlignment);
    }

    glCreateBuffers(1, &_handle.id);
    glNamedBufferStorage(_handle, size, nullptr, GL_DYNAMIC_STORAGE_BIT);

    _localBuffer.resize(size);
}

GrowableUbo::~GrowableUbo()
{
    if (_handle.id != 0)
    {
        glDeleteBuffers(1, &_handle.id);
    }
}

void GrowableUbo::bindIndex(u32 i, gl::Int binding) const
{
    ENGINE_ASSERT(_handle != 0, "Trying to bind to uninitialized buffer");
    glBindBufferRange(GL_UNIFORM_BUFFER, binding, _handle, i * _stride, _elemSize);
}

void GrowableUbo::resize(u32 newSize)
{
    if (_handle.id != 0)
    {
        glDeleteBuffers(1, &_handle.id);
    }

    glCreateBuffers(1, &_handle.id);
    glNamedBufferStorage(_handle, newSize, nullptr, GL_DYNAMIC_STORAGE_BIT);

    _localBuffer.resize(newSize);

    _size = newSize;

    spdlog::debug("Resizing uniform buffer (id:{}) to {:b}.", _handle, FormattableBytes{newSize});
}

void GrowableUbo::upload() const
{
    glNamedBufferSubData(_handle, 0, _localBuffer.size(), _localBuffer.data());
}
