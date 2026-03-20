#pragma once

#include <datatype.h>
#include <fmt/format.h>

using namespace core;

template<typename Tag>
struct fmt::formatter<gl::Handle<Tag>> : formatter<gl::Uint>
{
    auto format(const gl::Handle<Tag>& handle, format_context& ctx) const
    {
        return formatter<gl::Uint>::format(handle.id, ctx);
    }
};

inline std::string_view glTypeToString(gl::enum_t type)
{
    switch (type)
    {
        case GL_FLOAT:        return "float";
        case GL_FLOAT_VEC2:   return "vec2";
        case GL_FLOAT_VEC3:   return "vec3";
        case GL_FLOAT_VEC4:   return "vec4";
        case GL_INT:          return "int";
        case GL_UNSIGNED_INT: return "uint";
        case GL_FLOAT_MAT4:   return "mat4";
        case GL_FLOAT_MAT3:   return "mat3";
        case GL_SAMPLER_2D:   return "sampler2D";
        case GL_SAMPLER_CUBE: return "samplerCube";
        default:              return "unsupported type";
    }
}