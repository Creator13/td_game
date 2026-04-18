#pragma once

#include <vector>
#include <tracy/Tracy.hpp>

#include "datatype.h"

namespace core
{
    class GraphicsBuffer
    {
        u32 _size;
        u32 _currentDataSize;

        gl::buffer_t _handle;
        std::vector<std::byte> _localBuffer;

    public:
        explicit GraphicsBuffer(u32 size);
        ~GraphicsBuffer();

        template<typename T>
        void append(T& data);

        template<std::ranges::input_range R>
            requires std::ranges::sized_range<R>
        void appendRange(R&& data);

        void clear();
        void upload();
        void bind(gl::Int binding) const;

    private:
        void resize(u32 newSize);
    };

    template<typename T>
    void GraphicsBuffer::append(T& data)
    {
        ZoneScoped

        const usize elemSize = sizeof(T);
        const u32 requiredSize = elemSize + _currentDataSize;

        if (requiredSize > _size)
        {
            resize(requiredSize * 1.5f);
        }

        std::memcpy(_localBuffer.data() + _currentDataSize, &data, elemSize);
        _currentDataSize = requiredSize;
    }

    template<std::ranges::input_range R>
        requires std::ranges::sized_range<R>
    void GraphicsBuffer::appendRange(R&& data)
    {
        ZoneScoped

        using T = std::ranges::range_value_t<R>;
        const usize elemSize = sizeof(T);
        const u32 requiredSize = elemSize * std::ranges::size(data) + _currentDataSize;

        if (requiredSize > _size)
        {
            resize(requiredSize * 1.5f);
        }

        i32 i = 0;
        for (const T& elem : data)
        {
            std::memcpy(_localBuffer.data() + _currentDataSize + i * elemSize, &elem, elemSize);
            i++;
        }

        _currentDataSize = requiredSize;
    }
}
