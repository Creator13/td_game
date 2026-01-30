#include "datatype.h"

#include <type_traits>
#include <glad/gl.h>

using namespace core;

// GL assertions
static_assert(sizeof(gl::Uint) == sizeof(GLuint));
static_assert(sizeof(gl::Int) == sizeof(GLint));
static_assert(sizeof(gl::Float) == sizeof(GLfloat));
static_assert(sizeof(gl::Double) == sizeof(GLdouble));
static_assert(sizeof(gl::Sizei) == sizeof(GLsizei));
static_assert(sizeof(gl::Bool) == sizeof(GLboolean));

static_assert(std::is_same_v<gl::Uint, GLuint>);
static_assert(std::is_same_v<gl::Int, GLint>);
static_assert(std::is_same_v<gl::Float, GLfloat>);
static_assert(std::is_same_v<gl::Double, GLdouble>);
static_assert(std::is_same_v<gl::Sizei, GLsizei>);
static_assert(std::is_same_v<gl::Bool, GLboolean>);
static_assert(std::is_same_v<gl::enum_t, GLenum>);

// GFX handles <-> opengl
static_assert(sizeof(gl::texture_t) == sizeof(GLuint));
static_assert(alignof(gl::texture_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::texture_t>);
static_assert(std::is_trivially_copyable_v<gl::texture_t>);

static_assert(sizeof(gl::shader_t) == sizeof(GLuint));
static_assert(alignof(gl::shader_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::shader_t>);
static_assert(std::is_trivially_copyable_v<gl::shader_t>);

static_assert(sizeof(gl::program_t) == sizeof(GLuint));
static_assert(alignof(gl::program_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::program_t>);
static_assert(std::is_trivially_copyable_v<gl::program_t>);

static_assert(sizeof(gl::buffer_t) == sizeof(GLuint));
static_assert(alignof(gl::buffer_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::buffer_t>);
static_assert(std::is_trivially_copyable_v<gl::buffer_t>);

static_assert(sizeof(gl::vert_arr_t) == sizeof(GLuint));
static_assert(alignof(gl::vert_arr_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::vert_arr_t>);
static_assert(std::is_trivially_copyable_v<gl::vert_arr_t>);

static_assert(sizeof(gl::framebuffer_t) == sizeof(GLuint));
static_assert(alignof(gl::framebuffer_t) == alignof(GLuint));
static_assert(std::is_standard_layout_v<gl::framebuffer_t>);
static_assert(std::is_trivially_copyable_v<gl::framebuffer_t>);

namespace _internal_datatype
{
    void verified() { }
}
