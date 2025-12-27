//
// Created by OpenAI Assistant on 25-12-30.
//

#ifndef SIMPLE3DMODELER_AABB_H
#define SIMPLE3DMODELER_AABB_H

#include <glm/glm.hpp>

struct AABB
{
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
    bool valid{false};

    void reset();
    void expand(const glm::vec3& p);
    void include(const AABB& other);
};

AABB transformAABB(const AABB& box, const glm::mat4& transform);
float distancePointAABB(const AABB& box, const glm::vec3& point);
bool sphereIntersectsAABB(const AABB& box, const glm::vec3& center, float radius);

#endif //SIMPLE3DMODELER_AABB_H
