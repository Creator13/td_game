#pragma once

#include <span>
#include <vector>

#include "datatype.h"
#include "core/Time.h"

namespace core::gfx
{
    class GrowableUbo
    {
        static inline gl::Int _glUboOffsetAlignment;

        u32 _size;
        usize _stride;
        usize _elemSize;

        gl::buffer_t _handle;
        std::vector<std::byte> _localBuffer;

    public:
        explicit GrowableUbo(u32 size);
        ~GrowableUbo();

        template<std::ranges::input_range R>
            requires std::ranges::sized_range<R>
        void alignAndUpload(R&& data);

        void bindIndex(u32 i, gl::Int binding) const;

    private:
        void resize(u32 newSize);
        void upload() const;
    };
}


template<std::ranges::input_range R>
    requires std::ranges::sized_range<R>
void core::gfx::GrowableUbo::alignAndUpload(R&& data)
{
    using T = std::ranges::range_value_t<R>;
    _elemSize = sizeof(T);
    _stride = (_elemSize + _glUboOffsetAlignment - 1) & ~(_glUboOffsetAlignment - 1);

    const u32 alignedSize = _stride * std::ranges::size(data);
    if (alignedSize > _size)
    {
        resize(std::ranges::size(data) * 1.5 * _stride);
    }

    i32 i = 0;
    for (const T& elem : data)
    {
        std::memcpy(_localBuffer.data() + i * _stride, &elem, _elemSize);
        i++;
    }

    upload();
}
