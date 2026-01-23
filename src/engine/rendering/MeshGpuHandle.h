#pragma once
#include "datatype.h"

namespace core::gpu
{
    struct MeshGpuHandle
    {
        gl::vert_arr_t vao = 0;
        gl::buffer_t vbo = 0, ebo = 0;
        u32 indexCount = 0;
    };
}
