//
// Created by Remilia on 25-12-19.
//

#ifndef MODEL_H
#define MODEL_H

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class Mesh;

struct Model
{
    struct Submesh
    {
        std::string name;
        std::shared_ptr<Mesh> mesh;
        glm::mat4 local{1.0f};
    };

    std::string sourcePath;
    std::vector<Submesh> submeshes;
};

#endif //MODEL_H
