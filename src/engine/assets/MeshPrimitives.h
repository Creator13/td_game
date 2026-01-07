#pragma once

#include "rendering/Mesh.h"

namespace assets::mesh_primitives
{
    inline constexpr graphics::Vertex CUBE_VERTICES[] =
    {
        {math::vec3(-0.5f, -0.5f, -0.5f), math::vec3(0, 1, 0)},
        {math::vec3(0.5f, -0.5f, -0.5f), math::vec3(0, 1, 0)},
        {math::vec3(0.5f, -0.5f, 0.5f), math::vec3(0, 1, 0)},
        {math::vec3(-0.5f, -0.5f, 0.5f), math::vec3(0, 1, 0)},
        {math::vec3(0.5f, -0.5f, -0.5f), math::vec3(1, 0, 0)},
        {math::vec3(0.5f, 0.5f, -0.5f), math::vec3(1, 0, 0)},
        {math::vec3(0.5f, 0.5f, 0.5f), math::vec3(1, 0, 0)},
        {math::vec3(0.5f, -0.5f, 0.5f), math::vec3(1, 0, 0)},
        {math::vec3(0.5f, 0.5f, -0.5f), math::vec3(0, -1, 0)},
        {math::vec3(-0.5f, 0.5f, -0.5f), math::vec3(0, -1, 0)},
        {math::vec3(-0.5f, 0.5f, 0.5f), math::vec3(0, -1, 0)},
        {math::vec3(0.5f, 0.5f, 0.5f), math::vec3(0, -1, 0)},
        {math::vec3(-0.5f, 0.5f, -0.5f), math::vec3(-1, 0, 0)},
        {math::vec3(-0.5f, -0.5f, -0.5f), math::vec3(-1, 0, 0)},
        {math::vec3(-0.5f, -0.5f, 0.5f), math::vec3(-1, 0, 0)},
        {math::vec3(-0.5f, 0.5f, 0.5f), math::vec3(-1, 0, 0)},
        {math::vec3(-0.5f, 0.5f, -0.5f), math::vec3(0, 0, -1)},
        {math::vec3(0.5f, 0.5f, -0.5f), math::vec3(0, 0, -1)},
        {math::vec3(0.5f, -0.5f, -0.5f), math::vec3(0, 0, -1)},
        {math::vec3(-0.5f, -0.5f, -0.5f), math::vec3(0, 0, -1)},
        {math::vec3(-0.5f, -0.5f, 0.5f), math::vec3(0, 0, 1)},
        {math::vec3(0.5f, -0.5f, 0.5f), math::vec3(0, 0, 1)},
        {math::vec3(0.5f, 0.5f, 0.5f), math::vec3(0, 0, 1)},
        {math::vec3(-0.5f, 0.5f, 0.5f), math::vec3(0, 0, 1)},
    };
    inline constexpr uint32_t CUBE_INDICES[] =
    {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20,
    };
    inline constexpr math::AABB CUBE_BOUNDS = graphics::boundsFromVertices(CUBE_VERTICES);

    inline constexpr graphics::Vertex QUAD_VERTICES[] =
    {
        {math::vec3(0, 0, 0), math::vec3(0, 1, 0)},
        {math::vec3(1, 0, 0), math::vec3(0, 1, 0)},
        {math::vec3(1, 0, 1), math::vec3(0, 1, 0)},
        {math::vec3(0, 0, 1), math::vec3(0, 1, 0)},
    };
    inline constexpr uint32_t QUAD_INDICES[] =
    {
        0, 1, 2, 2, 3, 0
    };
    inline constexpr math::AABB QUAD_BOUNDS = graphics::boundsFromVertices(QUAD_VERTICES);
};
