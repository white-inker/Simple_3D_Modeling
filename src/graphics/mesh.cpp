//
// Created by Remilia on 25-12-18.
//
#include <graphics/mesh.h>
#include <utility>

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<std::uint32_t> &indices)
{
    createOnGPU(vertices, &indices);
}

Mesh::Mesh(const std::vector<Vertex> &vertices)
{
    createOnGPU(vertices, nullptr);
}

Mesh::~Mesh()
{
    destroy();
}

Mesh::Mesh(Mesh &&other) noexcept
{
    *this = std::move(other);
}

Mesh &Mesh::operator=(Mesh &&other) noexcept
{
    if (this == &other) return *this;

    destroy();

    vao = other.vao;
    other.vao = 0;

    vbo = other.vbo;
    other.vbo = 0;

    ebo = other.ebo;
    other.ebo = 0;

    vertexCount = other.vertexCount;
    other.vertexCount = 0;

    indexCount = other.indexCount;
    other.indexCount = 0;

    return *this;
}

void Mesh::destroy()
{
    if (ebo)
    {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

void Mesh::createOnGPU(const std::vector<Vertex> &vertices,
                       const std::vector<std::uint32_t> *indices)
{
    vertexCount = static_cast<GLsizei>(vertices.size());
    indexCount = indices ? static_cast<GLsizei>(indices->size()) : 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(),
                 GL_STATIC_DRAW);

    if (indices && !indices->empty())
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices->size() * sizeof(std::uint32_t)),
                     indices->data(),
                     GL_STATIC_DRAW);
    }

    // layout:
    // location 0: position (vec3)
    // location 1: normal   (vec3)
    // location 2: uv       (vec2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, position)));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void *>(offsetof(Vertex, uv)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Mesh::draw(GLenum primitive) const
{
    if (vao == 0) return;

    glBindVertexArray(vao);

    if (indexCount > 0)
        glDrawElements(primitive, indexCount, GL_UNSIGNED_INT, nullptr);
    else
        glDrawArrays(primitive, 0, vertexCount);

    glBindVertexArray(0);
}
