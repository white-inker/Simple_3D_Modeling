//
// Created by Remilia on 25-12-19.
//

#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <geometry/aabb.h>

class Mesh;
class Material;

struct Model
{
    struct Submesh
    {
        std::string name;
        std::shared_ptr<Mesh> mesh;
        glm::mat4 local{1.0f};
        std::shared_ptr<Material> material;
        AABB localBounds;
    };

    std::string sourcePath;
    std::vector<Submesh> submeshes;
    AABB bounds;
    bool hasMaterial() const
    {
        for (const auto& submesh : submeshes)
        {
            if (submesh.material == nullptr)
                return false;
        }
        return true;
    }
};

#endif //MODEL_H
