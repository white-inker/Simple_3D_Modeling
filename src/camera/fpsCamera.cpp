//
// Created by Omari on 25-12-17.
//

#include <camera/fpsCamera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

static float clampPitchFps(float pitchDeg)
{
    return std::clamp(pitchDeg, -89.0f, 89.0f);
}

FpsCamera::FpsCamera() = default;

void FpsCamera::setPosition(const glm::vec3& p)
{
    m_position = p;
    m_viewDirty = true;
}

void FpsCamera::setYawPitch(float yawDeg, float pitchDeg)
{
    m_yawDeg = yawDeg;
    m_pitchDeg = clampPitchFps(pitchDeg);
    m_viewDirty = true;
}

glm::vec3 FpsCamera::getForward() const
{
    const float yaw = glm::radians(m_yawDeg);
    const float pitch = glm::radians(m_pitchDeg);

    glm::vec3 f;
    f.x = cosf(pitch) * cosf(yaw);
    f.y = sinf(pitch);
    f.z = cosf(pitch) * sinf(yaw);
    return glm::normalize(f);
}

glm::vec3 FpsCamera::getRight() const
{
    constexpr glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    return glm::normalize(glm::cross(getForward(), worldUp));
}

glm::vec3 FpsCamera::getUp() const
{
    return glm::normalize(glm::cross(getRight(), getForward()));
}

void FpsCamera::moveWorld(const glm::vec3& delta)
{
    m_position += delta;
    m_viewDirty = true;
}

void FpsCamera::rotate(float deltaYawDeg, float deltaPitchDeg)
{
    m_yawDeg += deltaYawDeg;
    m_pitchDeg = clampPitchFps(m_pitchDeg + deltaPitchDeg);
    m_viewDirty = true;
}

const glm::mat4& FpsCamera::getViewMatrix() const
{
    if (m_viewDirty) updateView();
    return m_view;
}

void FpsCamera::updateView() const
{
    const glm::vec3 target = m_position + getForward();
    m_view = glm::lookAt(m_position, target, glm::vec3(0,1,0));
    m_viewDirty = false;
}
