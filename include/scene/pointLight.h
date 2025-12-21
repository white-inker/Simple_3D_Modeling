//
// Created by Remilia on 25-12-18.
//

#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include <scene/light.h>

class PointLight final : public Light
{
public:
    LightType type() const override { return LightType::Point; }

    glm::vec3 position{0.0f, 0.0f, 0.0f};

    // 用于 shader 衰减/截断（range 内有效）
    float range = 10.0f;
};

#endif //POINTLIGHT_H
