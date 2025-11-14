#include <glad/glad.h>
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

graphics::Mesh::Mesh(const std::vector<Vertex>& vertices,const std::vector<uint32_t>& indices)
    : vertices(vertices), indices(indices)
{
    uploadToGpu();
}

graphics::Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices)
    : vertices(std::move(vertices)), indices(std::move(indices))
{
    uploadToGpu();
}

graphics::Mesh::~Mesh()
{
    const GLuint buffers[] = {vbo, ebo};
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(2, buffers);
}

void graphics::Mesh::constructBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Vertex.position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Vertex.normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void graphics::Mesh::uploadToGpu()
{
    if (vao == 0)
    {
        constructBuffers();
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
