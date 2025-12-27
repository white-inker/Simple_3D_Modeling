//
// Created by Remilia on 25-12-18.
//

#ifndef MESH_H
#define MESH_H

#include <utils/common.h>
#include <cstdint>
#include <vector>

class Mesh
{
public:
    struct Vertex
    {
        glm::vec3 position{0,0,0};
        glm::vec3 normal{0,1,0};
        glm::vec2 uv{0,0};
    };

public:
    Mesh() = default;

    // 使用顶点+索引创建
    explicit Mesh(const std::vector<Vertex>& vertices,
         const std::vector<std::uint32_t>& indices);

    // 仅顶点
    explicit Mesh(const std::vector<Vertex>& vertices);

    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    // 绘制
    void draw(GLenum primitive = GL_TRIANGLES) const;

    // 给 Renderer 用的访问器
    GLuint getVao() const { return vao; }
    GLsizei getIndexCount() const { return indexCount; }
    GLsizei getVertexCount() const { return vertexCount; }
    bool hasIndices() const { return indexCount > 0; }
    const std::vector<Vertex>& getCpuVertices() const { return cpuVertices; }
    const std::vector<std::uint32_t>& getCpuIndices() const { return cpuIndices; }

private:
    void createOnGPU(const std::vector<Vertex>& vertices,
                     const std::vector<std::uint32_t>* indices);

    void destroy();

private:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;

    GLsizei vertexCount = 0;
    GLsizei indexCount  = 0;

    std::vector<Vertex> cpuVertices;
    std::vector<std::uint32_t> cpuIndices;
};


#endif //MESH_H
