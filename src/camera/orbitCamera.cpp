//
// Created by Omari on 25-12-17.
//

#include <camera/orbitCamera.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

static float clampPitch(float pitchDeg)
{
    // 防止翻转
    return std::clamp(pitchDeg, -89.0f, 89.0f);
}

OrbitCamera::OrbitCamera() = default;

void OrbitCamera::setTarget(const glm::vec3& t)
{
    m_target = t;
    m_viewDirty = true;
}

void OrbitCamera::setDistance(float d)
{
    m_distance = std::clamp(d, m_minDistance, m_maxDistance);
    m_viewDirty = true;
}

void OrbitCamera::setYawPitch(float yawDeg, float pitchDeg)
{
    m_yawDeg = yawDeg;
    m_pitchDeg = clampPitch(pitchDeg);
    m_viewDirty = true;
}

glm::vec3 OrbitCamera::getForward() const
{
    // forward：从相机位置指向 target
    const float yaw   = glm::radians(m_yawDeg);
    const float pitch = glm::radians(m_pitchDeg);

    glm::vec3 forward;
    forward.x = cosf(pitch) * cosf(yaw);
    forward.y = sinf(pitch);
    forward.z = cosf(pitch) * sinf(yaw);
    return glm::normalize(forward);
}

const glm::vec3 OrbitCamera::getPosition() const
{
    // position = target - forward * distance
    return m_target - getForward() * m_distance;
}

void OrbitCamera::orbit(float deltaYawDeg, float deltaPitchDeg)
{
    m_yawDeg += deltaYawDeg;
    m_pitchDeg = clampPitch(m_pitchDeg + deltaPitchDeg);
    m_viewDirty = true;
}

void OrbitCamera::pan(float deltaX, float deltaY)
{
    // deltaX/deltaY：由 controller 做像素->世界缩放
    const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    const glm::vec3 fwd = getForward();
    const glm::vec3 right = glm::normalize(glm::cross(fwd, worldUp));
    const glm::vec3 up = glm::normalize(glm::cross(right, fwd));

    // 拖动鼠标向右 -> 场景向右（target向左），这里按你习惯可调符号
    m_target += (-right * deltaX) + (up * deltaY);
    m_viewDirty = true;
}

void OrbitCamera::zoom(float delta)
{
    // delta > 0：缩近 or 缩远由 controller 定义；这里用减小距离实现“zoom in”
    setDistance(m_distance - delta);
}

const glm::mat4& OrbitCamera::getViewMatrix() const
{
    if (m_viewDirty) updateView();
    return m_view;
}

void OrbitCamera::updateView() const
{
    const glm::vec3 pos = getPosition();
    const glm::vec3 up(0.0f, 1.0f, 0.0f);
    m_view = glm::lookAt(pos, m_target, up);
    m_viewDirty = false;
}
