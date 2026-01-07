#include <rendering/Mesh.h>

using namespace graphics;
using namespace math;

void Mesh::recalculateBounds()
{
    bounds = boundsFromVertices(vertices);
}
