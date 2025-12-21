//
// Created by Remilia on 25-12-20.
//

#ifndef PRIMITIVEFACTORY_H
#define PRIMITIVEFACTORY_H

#include <memory>
#include <glm/glm.hpp>

class Mesh;
struct Model;

enum class PrimitiveType
{
    Cube,       // 立方体
    Sphere,     // 球
    Cylinder,   // 圆柱
    Cone,       // 圆锥
    Prism,      // 多面棱柱
    Frustum     // 多面棱台
};

struct PrimitiveDesc
{
    PrimitiveType type = PrimitiveType::Cube;

    // Cube
    glm::vec3 size{1.0f, 1.0f, 1.0f};

    float radius = 0.5f;     // Sphere, Cylinder
    int slices = 32; // 经线分段
    int stacks = 16; // 纬线分段

    // Cylinder / Cone / Prism / Frustum
    float height = 1.0f;
    int sides = 16;     // 圆周/多边形边数（>=3）
    bool caps = true;   // 是否顶面和底面

    // Frustum（棱台）/ Cone（圆锥可视作 topRadius=0 的棱台）
    float topRadius = 0.25f;
    float bottomRadius = 0.5f;

    // shading hint
    // 对 Prism/Frustum 建议 false（硬边）
    bool smooth = true;
};

class PrimitiveFactory
{
public:
    static std::shared_ptr<Model> CreateModel(const PrimitiveDesc& d);

    static std::shared_ptr<Mesh> CreateMesh(const PrimitiveDesc& d);

private:
    PrimitiveFactory() = default;
};


#endif //PRIMITIVEFACTORY_H
