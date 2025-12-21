//
// Created by Omari on 25-12-17.
//

#include <camera/perspectiveCamera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

void PerspectiveCamera::setAspect(int framebufferWidth, int framebufferHeight)
{
    framebufferHeight = std::max(1, framebufferHeight);
    m_aspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);
    markProjDirty();
}

void PerspectiveCamera::setPerspective(float fovYDegrees, float zNear, float zFar)
{
    m_fovYDeg = fovYDegrees;
    m_near = zNear;
    m_far  = zFar;
    markProjDirty();
}

const glm::mat4& PerspectiveCamera::getProjectionMatrix() const
{
    if (m_projDirty) updateProjection();
    return m_proj;
}

void PerspectiveCamera::updateProjection() const
{
    m_proj = glm::perspective(glm::radians(m_fovYDeg), m_aspect, m_near, m_far);
    m_projDirty = false;
}