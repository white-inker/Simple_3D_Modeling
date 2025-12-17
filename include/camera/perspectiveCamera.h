//
// Created by Omari on 25-12-17.
//

#ifndef PERSPECTIVECAMERA_H
#define PERSPECTIVECAMERA_H

#include <glm/glm.hpp>

class PerspectiveCamera
{
public:
    virtual ~PerspectiveCamera() = default;

    // viewport -> 影响 aspect
    void setViewportSize(int framebufferWidth, int framebufferHeight);

    // 透视参数
    void setPerspective(float fovYDegrees, float zNear, float zFar);

    float getFovYDegrees() const { return m_fovYDeg; }
    float getAspect() const { return m_aspect; }
    float getNear() const { return m_near; }
    float getFar() const { return m_far; }

    virtual const glm::mat4& getViewMatrix() const = 0;

    const glm::mat4& getProjectionMatrix() const;

protected:
    void markProjDirty() const { m_projDirty = true; }

private:
    void updateProjection() const;

protected:
    float m_fovYDeg = 45.0f;
    float m_aspect  = 4.0f / 3.0f;
    float m_near    = 0.1f;
    float m_far     = 100.0f;

    mutable glm::mat4 m_proj{1.0f};
    mutable bool m_projDirty = true;
};

#endif //PERSPECTIVECAMERA_H
