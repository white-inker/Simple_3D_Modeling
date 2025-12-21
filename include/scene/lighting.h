//
// Created by Remilia on 25-12-18.
//

#ifndef LIGHTING_H
#define LIGHTING_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include <scene/directionalLight.h>
#include <scene/pointLight.h>

class Lighting
{
public:
    // 环境光
    glm::vec3 ambient{0.03f, 0.03f, 0.03f};

    // 方向光 0/1
    void setDirectional(std::unique_ptr<DirectionalLight> light) { dir = std::move(light);}

    void clearDirectional() { dir.reset(); }

    DirectionalLight* directional() { return dir.get(); }
    const DirectionalLight* directional() const { return dir.get(); }

    // 点光（多个）
    PointLight& addPointLight()
    {
        points.emplace_back(std::make_unique<PointLight>());
        return *points.back();
    }

    std::vector<std::unique_ptr<PointLight>>& pointLights() { return points; }
    const std::vector<std::unique_ptr<PointLight>>& pointLights() const { return points; }

private:
    std::unique_ptr<DirectionalLight> dir;
    std::vector<std::unique_ptr<PointLight>> points;
};

#endif //LIGHTING_H
