//
// Created by Omari on 25-12-17.
//

#ifndef ORBITCAMERA_H
#define ORBITCAMERA_H

#include <camera/perspectiveCamera.h>
#include <glm/glm.hpp>

class OrbitCamera final : public PerspectiveCamera
{
public:
    OrbitCamera();

    void setTarget(const glm::vec3& t);
    void setDistance(float d);
    void setYawPitch(float yawDeg, float pitchDeg);

    const glm::vec3& getTarget() const { return m_target; }
    float getDistance() const { return m_distance; }
    float getYawDeg() const { return m_yawDeg; }
    float getPitchDeg() const { return m_pitchDeg; }

    const glm::vec3 getPosition() const override; // 根据 target+yaw/pitch+distance推出来
    glm::vec3 getForward() const;  // position -> target 方向

    void orbit(float deltaYawDeg, float deltaPitchDeg);
    void pan(float deltaX, float deltaY);     // 屏幕空间像素/任意单位
    void zoom(float delta);                  // 正数放大/缩小由 controller 定义

    const glm::mat4& getViewMatrix() const override;

private:
    void updateView() const;

private:
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    float m_distance = 5.0f;

    float m_yawDeg   = 90.0f; // degree
    float m_pitchDeg = 0.0f;

    float m_minDistance = 0.2f;
    float m_maxDistance = 200.0f;

    mutable glm::mat4 m_view{1.0f};
    mutable bool m_viewDirty = true;
};


#endif //ORBITCAMERA_H
