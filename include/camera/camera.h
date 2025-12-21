//
// Created by Remilia on 25-12-19.
//

#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera
{
public:
    virtual ~Camera() = default;
    virtual const glm::mat4& getViewMatrix() const = 0;
    virtual const glm::mat4& getProjectionMatrix() const = 0;
    virtual const glm::vec3 getPosition() const = 0;
    //virtual void setAspect(float aspect);
    virtual void setAspect(int w, int h) {}
    //virtual const glm::quat getRotation() const = 0;
    //virtual void setPosition(const glm::vec3& p) = 0;
    //virtual void setRotation(const glm::quat& q) = 0;
    //virtual glm::vec3 forward() const = 0;
    //virtual glm::vec3 right()   const = 0;
    //virtual glm::vec3 up()      const = 0;
};

#endif //CAMERA_H
