#pragma once

#include <vector>

#include "math/bounds.h"
#include "math/vec3.h"

struct Mesh
{
    std::vector<math::vec3> positions;
    std::vector<math::vec3> normals;
    std::vector<math::vec2> texcoords;
    std::vector<uint16_t> indices;

    math::bounds3d bounds;
};
