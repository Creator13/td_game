#pragma once

#include <cstdint>

namespace core
{
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using f16 = short;
    using f32 = float;
    using f64 = double;

    using usize = size_t;
    using isize = ptrdiff_t;
}

namespace core::gl
{
    using Uint = uint32_t;
    using Int = int32_t;
    using Sizei = int32_t;
    using Bool = uint8_t;
    using Float = float;
    using Double = double;

    using enum_t = uint32_t;
}

namespace core::gl
{
    template<typename>
    struct Handle
    {
        Uint id;

        constexpr Handle() : id(0) { }
        constexpr Handle(Uint id) : id(id) { }

        operator gl::Uint() const { return id; }
        explicit constexpr operator bool() const { return id != 0; }

        template<typename OtherTag>
        Handle(Handle<OtherTag>) = delete;

        template<typename OtherTag>
        Handle& operator=(Handle<OtherTag>) = delete;
    };

    struct TextureTag { };

    struct ShaderTag { };

    struct ProgramTag { };

    struct BufferTag { };

    struct VertexArrayTag { };

    struct FrameBufferTag { };

    using texture_t = Handle<TextureTag>;
    using shader_t = Handle<ShaderTag>;
    using program_t = Handle<ProgramTag>;
    using buffer_t = Handle<BufferTag>;
    using vert_arr_t = Handle<VertexArrayTag>;
    using framebuffer_t = Handle<FrameBufferTag>;
}
