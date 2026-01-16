#include <rendering/Mesh.h>

using namespace core;
using namespace math;

void Mesh::recalculateBounds()
{
    bounds = boundsFromVertices(vertices);
}
