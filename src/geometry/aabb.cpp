//
// Created by OpenAI Assistant on 25-12-30.
//

#include <geometry/aabb.h>

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

void AABB::reset()
{
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::lowest());
    valid = false;
}

void AABB::expand(const glm::vec3& p)
{
    if (!valid)
    {
        min = max = p;
        valid = true;
        return;
    }
    min = glm::min(min, p);
    max = glm::max(max, p);
}

void AABB::include(const AABB& other)
{
    if (!other.valid) return;
    if (!valid)
    {
        *this = other;
        return;
    }
    min = glm::min(min, other.min);
    max = glm::max(max, other.max);
}

static glm::vec3 transformPoint(const glm::mat4& m, const glm::vec3& p)
{
    const glm::vec4 t = m * glm::vec4(p, 1.0f);
    return glm::vec3(t) / t.w;
}

AABB transformAABB(const AABB& box, const glm::mat4& transform)
{
    if (!box.valid) return box;

    const glm::vec3 corners[8] = {
        {box.min.x, box.min.y, box.min.z},
        {box.max.x, box.min.y, box.min.z},
        {box.min.x, box.max.y, box.min.z},
        {box.min.x, box.min.y, box.max.z},
        {box.max.x, box.max.y, box.min.z},
        {box.max.x, box.min.y, box.max.z},
        {box.min.x, box.max.y, box.max.z},
        {box.max.x, box.max.y, box.max.z}
    };

    AABB res;
    res.reset();
    for (const auto& c : corners)
        res.expand(transformPoint(transform, c));

    return res;
}

float distancePointAABB(const AABB& box, const glm::vec3& point)
{
    if (!box.valid) return std::numeric_limits<float>::infinity();

    glm::vec3 clamped = glm::clamp(point, box.min, box.max);
    return glm::length(point - clamped);
}

bool sphereIntersectsAABB(const AABB& box, const glm::vec3& center, float radius)
{
    const float dist = distancePointAABB(box, center);
    return dist <= radius;
}
