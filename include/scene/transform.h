//
// Created by Remilia on 25-12-18.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform
{
public:
    Transform();

    const glm::vec3& getPosition() const { return position; }
    const glm::quat& getRotation() const { return rotation; }
    const glm::vec3& getScale()    const { return scale; }

    void setPosition(const glm::vec3& p);
    void setRotation(const glm::quat& q);
    void setRotationEulerDeg(const glm::vec3& eulerDeg); // pitch(x), yaw(y), roll(z)
    void setScale(const glm::vec3& s);

    void translate(const glm::vec3& delta);
    void rotateEulerDeg(const glm::vec3& deltaEulerDeg);
    void rotateAxisDeg(const glm::vec3& axis, float angleDeg);

    const glm::mat4& modelMatrix() const;

private:
    void markDirty() const { m_dirty = true; }
    void updateModel() const;

private:
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f}; // w,x,y,z
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    mutable glm::mat4 model{1.0f};
    mutable bool m_dirty{true};
};


#endif //TRANSFORM_H
