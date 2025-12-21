//
// Created by Omari on 25-12-17.
//

#ifndef FPSCAMERA_H
#define FPSCAMERA_H

#include <camera/perspectiveCamera.h>
#include <glm/glm.hpp>

class FpsCamera final : public PerspectiveCamera
{
public:
    FpsCamera();

    void setPosition(const glm::vec3& p);
    void setYawPitch(float yawDeg, float pitchDeg);

    const glm::vec3 getPosition() const override { return m_position; }
    float getYawDeg() const { return m_yawDeg; }
    float getPitchDeg() const { return m_pitchDeg; }

    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

    // 操作（不读输入）
    void moveWorld(const glm::vec3& delta);
    void rotate(float deltaYawDeg, float deltaPitchDeg);

    const glm::mat4& getViewMatrix() const override;

private:
    void updateView() const;

private:
    glm::vec3 m_position{0.0f, 1.5f, 5.0f};
    float m_yawDeg   = -90.0f;
    float m_pitchDeg = 0.0f;

    mutable glm::mat4 m_view{1.0f};
    mutable bool m_viewDirty = true;
};


#endif //FPSCAMERA_H
