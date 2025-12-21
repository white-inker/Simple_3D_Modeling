//
// Created by Remilia on 25-12-18.
//

#ifndef FPSCAMERACONTROLLER_H
#define FPSCAMERACONTROLLER_H

#include <camera/cameraController.h>
#include <camera/fpsCamera.h>
#include <interaction/inputContext.h>

class FpsCameraController : public CameraController
{
public:
    explicit FpsCameraController(FpsCamera& cam) : m_cam(std::move(cam)) {}

    float moveSpeed = 5.0f;          // 单位/秒
    float fastMultiplier = 3.0f;
    float lookSensitivity = 0.05f;   // 度/像素
    float fovScrollStepDeg  = 3.0f;
    float fovMinDeg = 20.0f;
    float fovMaxDeg = 90.0f;

    bool  smoothFov = true;
    float fovSmoothSpeed = 18.0f;   // 越大越快
    float targetFovDeg = -1.0f;     // lazy init

    int lookMouseButton = GLFW_MOUSE_BUTTON_RIGHT;

    void update(const InputContext& input, float dt, void* ptr) override;
    Camera& getCamera() override { return m_cam;}

private:
    FpsCamera m_cam;
};


#endif //FPSCAMERACONTROLLER_H
