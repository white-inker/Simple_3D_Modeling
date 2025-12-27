//
// Created by Remilia on 25-12-18.
//

#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

enum class LightType
{
    Directional,
    Point
};

class Light
{
public:
    virtual ~Light() = default;
    virtual LightType type() const = 0;

    bool enabled = true;
    bool enableShadow = true;
    glm::vec3 color{1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
};

#endif //LIGHT_H
