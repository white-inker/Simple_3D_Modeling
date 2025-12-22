//
// Created by Remilia on 25-12-20.
//

#include <geometry/primitiveFactory.h>

#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <graphics/mesh.h>
#include <assets/model.h>

static int clampMin(int v, int mn) { return std::max(v, mn); }

static void appendIndexed(
    std::vector<Mesh::Vertex>& V,
    std::vector<std::uint32_t>& I,
    const std::vector<Mesh::Vertex>& vAdd,
    const std::vector<std::uint32_t>& iAdd)
{
    const std::uint32_t base = static_cast<std::uint32_t>(V.size());
    V.insert(V.end(), vAdd.begin(), vAdd.end());
    I.reserve(I.size() + iAdd.size());
    for (auto idx : iAdd) I.push_back(base + idx);
}

// -------------------- Cube --------------------
// 24 vertices (6 faces * 4), hard normals, simple UV
static void buildCube(const PrimitiveDesc& d,
                      std::vector<Mesh::Vertex>& outV,
                      std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();
    const glm::vec3 hs = d.size * 0.5f;

    struct Face { glm::vec3 n; glm::vec3 a,b,c,d; }; // quad a-b-c-d CCW
    const Face faces[6] = {
        // +X
        { {1,0,0}, {hs.x,-hs.y,-hs.z}, {hs.x,-hs.y, hs.z}, {hs.x, hs.y, hs.z}, {hs.x, hs.y,-hs.z} },
        // -X
        { {-1,0,0}, {-hs.x,-hs.y, hs.z}, {-hs.x,-hs.y,-hs.z}, {-hs.x, hs.y,-hs.z}, {-hs.x, hs.y, hs.z} },
        // +Y
        { {0,1,0}, {-hs.x,hs.y,-hs.z}, { hs.x,hs.y,-hs.z}, { hs.x,hs.y, hs.z}, {-hs.x,hs.y, hs.z} },
        // -Y
        { {0,-1,0}, {-hs.x,-hs.y, hs.z}, { hs.x,-hs.y, hs.z}, { hs.x,-hs.y,-hs.z}, {-hs.x,-hs.y,-hs.z} },
        // +Z
        { {0,0,1}, { hs.x,-hs.y,hs.z}, {-hs.x,-hs.y,hs.z}, {-hs.x, hs.y,hs.z}, { hs.x, hs.y,hs.z} },
        // -Z
        { {0,0,-1}, {-hs.x,-hs.y,-hs.z}, { hs.x,-hs.y,-hs.z}, { hs.x, hs.y,-hs.z}, {-hs.x, hs.y,-hs.z} },
    };

    outV.reserve(24);
    outI.reserve(36);

    for (int f=0; f<6; ++f)
    {
        const auto& F = faces[f];
        const std::uint32_t base = static_cast<std::uint32_t>(outV.size());

        Mesh::Vertex v0{F.a, F.n, {0,0}};
        Mesh::Vertex v1{F.b, F.n, {1,0}};
        Mesh::Vertex v2{F.c, F.n, {1,1}};
        Mesh::Vertex v3{F.d, F.n, {0,1}};
        outV.push_back(v0); outV.push_back(v1); outV.push_back(v2); outV.push_back(v3);

        // two triangles: 0-1-2, 0-2-3
        outI.push_back(base+0); outI.push_back(base+1); outI.push_back(base+2);
        outI.push_back(base+0); outI.push_back(base+2); outI.push_back(base+3);
    }
}

// -------------------- Sphere (UV sphere) --------------------
static void buildSphere(const PrimitiveDesc& d,
                        std::vector<Mesh::Vertex>& outV,
                        std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();

    const float r = std::max(0.0001f, d.radius);
    const int slices = clampMin(d.slices, 3);
    const int stacks = clampMin(d.stacks, 2);

    // vertices: (stacks+1) * (slices+1)
    outV.reserve((stacks + 1) * (slices + 1));
    outI.reserve(stacks * slices * 6);

    for (int i = 0; i <= stacks; ++i)
    {
        float v = static_cast<float>(i) / static_cast<float>(stacks);        // 0..1
        float theta = v * glm::pi<float>();        // 0..PI
        float y = std::cos(theta);
        float sinT = std::sin(theta);

        for (int j = 0; j <= slices; ++j)
        {
            float u = static_cast<float>(j) / static_cast<float>(slices);    // 0..1
            float phi = u * glm::two_pi<float>();  // 0..2PI

            float x = sinT * std::cos(phi);
            float z = sinT * std::sin(phi);

            glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
            glm::vec3 p = n * r;

            Mesh::Vertex vert;
            vert.position = p;
            vert.normal = n;
            vert.uv = {u, 1.0f - v};
            outV.push_back(vert);
        }
    }

    auto idx = [&](int i, int j) -> std::uint32_t {
        return static_cast<std::uint32_t>(i * (slices + 1) + j);
    };

    for (int i = 0; i < stacks; ++i)
    {
        for (int j = 0; j < slices; ++j)
        {
            std::uint32_t i0 = idx(i, j);
            std::uint32_t i1 = idx(i + 1, j);
            std::uint32_t i2 = idx(i + 1, j + 1);
            std::uint32_t i3 = idx(i, j + 1);

            outI.push_back(i0); outI.push_back(i1); outI.push_back(i2);
            outI.push_back(i0); outI.push_back(i2); outI.push_back(i3);
        }
    }
}

// -------------------- Disk (cap helper) --------------------
static void appendDisk(float y, float radius, int sides, bool top,
                       std::vector<Mesh::Vertex>& outV,
                       std::vector<std::uint32_t>& outI)
{
    if (radius <= 0.00001f) return;
    sides = clampMin(sides, 3);

    const glm::vec3 n = top ? glm::vec3(0,1,0) : glm::vec3(0,-1,0);

    const std::uint32_t base = static_cast<std::uint32_t>(outV.size());

    // center
    outV.push_back({ {0,y,0}, n, {0.5f, 0.5f} });

    // ring vertices (sides+1 to close UV seam)
    for (int i=0; i<=sides; ++i)
    {
        float u = static_cast<float>(i) / static_cast<float>(sides);
        float a = u * glm::two_pi<float>();
        float x = std::cos(a) * radius;
        float z = std::sin(a) * radius;

        // disk UV mapping
        glm::vec2 uv(0.5f + x / (2.0f*radius), 0.5f + z / (2.0f*radius));
        outV.push_back({ {x,y,z}, n, uv });
    }

    for (int i=0; i<sides; ++i)
    {
        std::uint32_t c = base + 0;
        std::uint32_t r0 = base + 1 + i;
        std::uint32_t r1 = base + 1 + i + 1;

        if (top)
        {
            // center, r0, r1 (CCW from +Y)
            outI.push_back(c); outI.push_back(r0); outI.push_back(r1);
        }
        else
        {
            // center, r1, r0 (CCW from -Y)
            outI.push_back(c); outI.push_back(r1); outI.push_back(r0);
        }
    }
}

// -------------------- Cylinder --------------------
static void buildCylinder(const PrimitiveDesc& d,
                          std::vector<Mesh::Vertex>& outV,
                          std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();

    const float r = std::max(0.0001f, d.radius);
    const float h = d.height;
    const int sides = clampMin(d.sides, 3);
    const float yTop = +h * 0.5f;
    const float yBot = -h * 0.5f;

    // side vertices: (sides+1)*2
    const auto baseSide = static_cast<std::uint32_t>(outV.size());
    outV.reserve((sides+1)*2);

    for (int i=0; i<=sides; ++i)
    {
        float u = static_cast<float>(i) / static_cast<float>(sides);
        float a = u * glm::two_pi<float>();
        float x = std::cos(a) * r;
        float z = std::sin(a) * r;

        glm::vec3 n = glm::normalize(glm::vec3(x, 0.0f, z));

        // bottom
        outV.push_back({ {x, yBot, z}, n, {u, 0.0f} });
        // top
        outV.push_back({ {x, yTop, z}, n, {u, 1.0f} });
    }

    // side indices
    for (int i=0; i<sides; ++i)
    {
        std::uint32_t i0 = baseSide + static_cast<std::uint32_t>(i * 2 + 0); // bot i
        std::uint32_t i1 = baseSide + static_cast<std::uint32_t>(i * 2 + 1); // top i
        std::uint32_t i2 = baseSide + static_cast<std::uint32_t>((i + 1) * 2 + 0); // bot i+1
        std::uint32_t i3 = baseSide + static_cast<std::uint32_t>((i + 1) * 2 + 1); // top i+1

        // two triangles (CCW from outside)
        outI.push_back(i1); outI.push_back(i0); outI.push_back(i2);
        outI.push_back(i1); outI.push_back(i2); outI.push_back(i3);
    }

    // caps
    if (d.caps)
    {
        appendDisk(yTop, r, sides, true,  outV, outI);
        appendDisk(yBot, r, sides, false, outV, outI);
    }
}

// -------------------- Cone --------------------
static void buildCone(const PrimitiveDesc& d,
                      std::vector<Mesh::Vertex>& outV,
                      std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();

    const float r = std::max(0.0001f, d.radius);
    const float h = d.height;
    const int sides = clampMin(d.sides, 3);

    const float yTop = +h * 0.5f;
    const float yBot = -h * 0.5f;

    // 为了 UV seam + 法线合理：每个 slice 有自己的 apex 顶点
    // 侧面：2*(sides+1) 顶点（base_i + apex_i）
    const std::uint32_t base = static_cast<std::uint32_t>(outV.size());
    outV.reserve((sides+1)*2);

    // cone slope normal approx: normalize(vec3(x, r/h, z))
    float slopeY = (h != 0.0f) ? (r / std::abs(h)) : 1.0f;

    for (int i=0; i<=sides; ++i)
    {
        float u = static_cast<float>(i) / static_cast<float>(sides);
        float a = u * glm::two_pi<float>();
        float x = std::cos(a) * r;
        float z = std::sin(a) * r;

        glm::vec3 n = glm::normalize(glm::vec3(x, slopeY * r, z));

        // base ring
        outV.push_back({ {x, yBot, z}, n, {u, 0.0f} });
        // apex (duplicate)
        outV.push_back({ {0, yTop, 0}, n, {u, 1.0f} });
    }

    for (int i=0; i<sides; ++i)
    {
        std::uint32_t b0 = base + static_cast<std::uint32_t>(i * 2 + 0);
        std::uint32_t a0 = base + static_cast<std::uint32_t>(i * 2 + 1);
        std::uint32_t b1 = base + static_cast<std::uint32_t>((i + 1) * 2 + 0);
        std::uint32_t a1 = base + static_cast<std::uint32_t>((i + 1) * 2 + 1);

        // 每个面用 (a0, b0, b1)
        outI.push_back(a0); outI.push_back(b0); outI.push_back(b1);
        // 可选：把 apex 也连一条（严格说不需要第二三角，因为是三角面）
        // 这里不加第二个三角
        (void)a1;
    }

    // base cap
    if (d.caps)
        appendDisk(yBot, r, sides, false, outV, outI);
}

// -------------------- Prism / Frustum helpers --------------------
static glm::vec3 faceNormal(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
{
    glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
    float len = glm::length(n);
    if (len < 1e-8f) return {0,1,0};
    return n / len;
}

// Prism: topRadius == bottomRadius
static void buildPrism(const PrimitiveDesc& d,
                       std::vector<Mesh::Vertex>& outV,
                       std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();

    const int sides = clampMin(d.sides, 3);
    const float r = std::max(0.0001f, d.radius);
    const float h = d.height;
    const float yTop = +h * 0.5f;
    const float yBot = -h * 0.5f;

    // sides: hard edges (each face has its own 4 vertices)
    for (int i=0; i<sides; ++i)
    {
        float u0 = static_cast<float>(i) / static_cast<float>(sides);
        float u1 = static_cast<float>(i + 1) / static_cast<float>(sides);

        float a0 = u0 * glm::two_pi<float>();
        float a1 = u1 * glm::two_pi<float>();

        glm::vec3 p00(std::cos(a0)*r, yBot, std::sin(a0)*r);
        glm::vec3 p01(std::cos(a0)*r, yTop, std::sin(a0)*r);
        glm::vec3 p10(std::cos(a1)*r, yBot, std::sin(a1)*r);
        glm::vec3 p11(std::cos(a1)*r, yTop, std::sin(a1)*r);

        // face normal (flat)
        glm::vec3 n = faceNormal(p01, p00, p10);

        std::uint32_t base = static_cast<std::uint32_t>(outV.size());
        outV.push_back({p00, n, {u0, 0.0f}});
        outV.push_back({p01, n, {u0, 1.0f}});
        outV.push_back({p11, n, {u1, 1.0f}});
        outV.push_back({p10, n, {u1, 0.0f}});

        outI.push_back(base+1); outI.push_back(base+0); outI.push_back(base+3);
        outI.push_back(base+1); outI.push_back(base+3); outI.push_back(base+2);
    }

    // caps: polygon fan
    if (d.caps)
    {
        // top
        {
            std::uint32_t base = static_cast<std::uint32_t>(outV.size());
            outV.push_back({{0,yTop,0}, {0,1,0}, {0.5f,0.5f}});

            std::vector<std::uint32_t> ring(sides);
            for (int i=0; i<sides; ++i)
            {
                float u = static_cast<float>(i) / static_cast<float>(sides);
                float a = u * glm::two_pi<float>();
                float x = std::cos(a)*r;
                float z = std::sin(a)*r;

                glm::vec2 uv(0.5f + x/(2*r), 0.5f + z/(2*r));
                ring[i] = static_cast<std::uint32_t>(outV.size());
                outV.push_back({{x,yTop,z},{0,1,0},uv});
            }

            for (int i=0; i<sides; ++i)
            {
                int j = (i+1) % sides;
                outI.push_back(base);
                outI.push_back(ring[i]);
                outI.push_back(ring[j]);
            }
        }

        // bottom
        {
            std::uint32_t base = static_cast<std::uint32_t>(outV.size());
            outV.push_back({{0,yBot,0}, {0,-1,0}, {0.5f,0.5f}});

            std::vector<std::uint32_t> ring(sides);
            for (int i=0; i<sides; ++i)
            {
                float u = static_cast<float>(i) / static_cast<float>(sides);
                float a = u * glm::two_pi<float>();
                float x = std::cos(a)*r;
                float z = std::sin(a)*r;

                glm::vec2 uv(0.5f + x/(2*r), 0.5f + z/(2*r));
                ring[i] = static_cast<std::uint32_t>(outV.size());
                outV.push_back({{x,yBot,z},{0,-1,0},uv});
            }

            for (int i=0; i<sides; ++i)
            {
                int j = (i+1) % sides;
                // reverse winding for bottom
                outI.push_back(base);
                outI.push_back(ring[j]);
                outI.push_back(ring[i]);
            }
        }
    }
}

static void buildFrustum(const PrimitiveDesc& d,
                         std::vector<Mesh::Vertex>& outV,
                         std::vector<std::uint32_t>& outI)
{
    outV.clear(); outI.clear();

    const int sides = clampMin(d.sides, 3);
    const float rt = std::max(0.0f, d.topRadius);
    const float rb = std::max(0.0f, d.bottomRadius);
    const float h  = d.height;
    const float yTop = +h * 0.5f;
    const float yBot = -h * 0.5f;

    // sides: hard edges (each face has its own 4 vertices)
    for (int i=0; i<sides; ++i)
    {
        float u0 = static_cast<float>(i) / static_cast<float>(sides);
        float u1 = static_cast<float>(i + 1) / static_cast<float>(sides);

        float a0 = u0 * glm::two_pi<float>();
        float a1 = u1 * glm::two_pi<float>();

        glm::vec3 b0(std::cos(a0)*rb, yBot, std::sin(a0)*rb);
        glm::vec3 t0(std::cos(a0)*rt, yTop, std::sin(a0)*rt);
        glm::vec3 b1(std::cos(a1)*rb, yBot, std::sin(a1)*rb);
        glm::vec3 t1(std::cos(a1)*rt, yTop, std::sin(a1)*rt);

        glm::vec3 n = faceNormal(t0, b0, b1);

        std::uint32_t base = static_cast<std::uint32_t>(outV.size());
        outV.push_back({b0, n, {u0, 0.0f}});
        outV.push_back({t0, n, {u0, 1.0f}});
        outV.push_back({t1, n, {u1, 1.0f}});
        outV.push_back({b1, n, {u1, 0.0f}});

        outI.push_back(base+1); outI.push_back(base+0); outI.push_back(base+3);
        outI.push_back(base+1); outI.push_back(base+3); outI.push_back(base+2);
    }

    // caps (optional)
    if (d.caps)
    {
        if (rt > 0.00001f)
        {
            // top cap
            std::uint32_t base = static_cast<std::uint32_t>(outV.size());
            outV.push_back({{0,yTop,0},{0,1,0},{0.5f,0.5f}});

            std::vector<std::uint32_t> ring(sides);
            for (int i=0; i<sides; ++i)
            {
                float u = static_cast<float>(i) / static_cast<float>(sides);
                float a = u * glm::two_pi<float>();
                float x = std::cos(a)*rt;
                float z = std::sin(a)*rt;
                glm::vec2 uv(0.5f + x/(2*rt), 0.5f + z/(2*rt));
                ring[i] = static_cast<std::uint32_t>(outV.size());
                outV.push_back({{x,yTop,z},{0,1,0},uv});
            }
            for (int i=0; i<sides; ++i)
            {
                int j = (i+1)%sides;
                outI.push_back(base);
                outI.push_back(ring[i]);
                outI.push_back(ring[j]);
            }
        }

        if (rb > 0.00001f)
        {
            // bottom cap
            std::uint32_t base = static_cast<std::uint32_t>(outV.size());
            outV.push_back({{0,yBot,0},{0,-1,0},{0.5f,0.5f}});

            std::vector<std::uint32_t> ring(sides);
            for (int i=0; i<sides; ++i)
            {
                float u = static_cast<float>(i) / static_cast<float>(sides);
                float a = u * glm::two_pi<float>();
                float x = std::cos(a)*rb;
                float z = std::sin(a)*rb;
                glm::vec2 uv(0.5f + x/(2*rb), 0.5f + z/(2*rb));
                ring[i] = static_cast<std::uint32_t>(outV.size());
                outV.push_back({{x,yBot,z},{0,-1,0},uv});
            }
            for (int i=0; i<sides; ++i)
            {
                int j = (i+1)%sides;
                outI.push_back(base);
                outI.push_back(ring[j]);
                outI.push_back(ring[i]);
            }
        }
    }
}

// -------------------- Factory --------------------

std::shared_ptr<Mesh> PrimitiveFactory::CreateMesh(const PrimitiveDesc& d)
{
    std::vector<Mesh::Vertex> V;
    std::vector<std::uint32_t> I;

    switch (d.type)
    {
    case PrimitiveType::Cube:
        buildCube(d, V, I);
        break;
    case PrimitiveType::Sphere:
        buildSphere(d, V, I);
        break;
    case PrimitiveType::Cylinder:
        buildCylinder(d, V, I);
        break;
    case PrimitiveType::Cone:
        buildCone(d, V, I);
        break;
    case PrimitiveType::Prism:
        buildPrism(d, V, I);
        break;
    case PrimitiveType::Frustum:
        buildFrustum(d, V, I);
        break;
    default:
        buildCube(d, V, I);
        break;
    }

    if (V.empty())
        return nullptr;

    return std::make_shared<Mesh>(V, I);
}

std::shared_ptr<Model> PrimitiveFactory::CreateModel(const PrimitiveDesc& d)
{
    const auto mesh = CreateMesh(d);
    if (!mesh) return nullptr;

    auto model = std::make_shared<Model>();
    model->sourcePath = "primitive";

    Model::Submesh sm;
    sm.name = "primitive";
    sm.mesh = mesh;
    sm.material = nullptr;
    sm.local = glm::mat4(1.0f);

    model->submeshes.push_back(std::move(sm));
    return model;
}
