//
// Created by Remilia on 25-12-18.
//

#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include <scene/light.h>

class DirectionalLight final : public Light
{
public:
    LightType type() const override { return LightType::Directional; }

    glm::vec3 direction{-0.2f, -1.0f, -0.3f};
};


#endif //DIRECTIONALLIGHT_H
