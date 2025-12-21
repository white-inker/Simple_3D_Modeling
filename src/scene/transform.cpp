//
// Created by Remilia on 25-12-18.
//
#include <scene/transform.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

Transform::Transform() = default;

void Transform::setPosition(const glm::vec3& p)
{
    position = p;
    markDirty();
}

void Transform::setRotation(const glm::quat& q)
{
    rotation = glm::normalize(q);
    markDirty();
}

void Transform::setRotationEulerDeg(const glm::vec3& eulerDeg)
{
    glm::vec3 rad = glm::radians(eulerDeg);
    rotation = glm::normalize(glm::quat(rad));
    markDirty();
}

void Transform::setScale(const glm::vec3& s)
{
    scale = s;
    markDirty();
}

void Transform::translate(const glm::vec3& delta)
{
    position += delta;
    markDirty();
}

void Transform::rotateEulerDeg(const glm::vec3& deltaEulerDeg)
{
    glm::vec3 rad = glm::radians(deltaEulerDeg);
    glm::quat dq = glm::quat(rad);
    rotation = glm::normalize(dq * rotation);
    markDirty();
}

void Transform::rotateAxisDeg(const glm::vec3& axis, float angleDeg)
{
    glm::quat dq = glm::angleAxis(glm::radians(angleDeg), glm::normalize(axis));
    rotation = glm::normalize(dq * rotation);
    markDirty();
}

const glm::mat4& Transform::modelMatrix() const
{
    if (m_dirty) updateModel();
    return model;
}

void Transform::updateModel() const
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 R = glm::toMat4(rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

    model = T * R * S;
    m_dirty = false;
}
