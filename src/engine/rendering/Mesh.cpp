#include <rendering/Mesh.h>

using namespace math;

// void render::mesh::addVertex(Mesh& mesh, const Vertex& vertex)
// {
//     mesh.indices.push_back(mesh.vertices.size());
//     mesh.vertices.push_back(vertex);
// }
//
// void render::mesh::addTri(Mesh& mesh, vec3 p0, vec3 p1, vec3 p2)
// {
//     const vec3 normal = normalize(cross(p1 - p0, p2 - p0));
//
//     addVertex(mesh, {p0, normal});
//     addVertex(mesh, {p1, normal});
//     addVertex(mesh, {p2, normal});
// }
//
// void render::mesh::addQuad(Mesh& mesh, vec3 p0, vec3 p1, vec3 p2, vec3 p3)
// {
//     // assuming vertices are in ccw order, then: tri0 = 0 -> 1 -> 2, tri1 = 0 -> 2 -> 3
//
//     const vec3 n1 = normalize(cross(p1 - p0, p2 - p0));
//     const vec3 n2 = normalize(cross(p2 - p0, p3 - p0));
//     const vec3 normal = (n1 + n2) * 0.5;
//
//     addVertex(mesh, {p0, normal});
//     addVertex(mesh, {p1, normal});
//     addVertex(mesh, {p2, normal});
//     addVertex(mesh, {p3, normal});
// }
