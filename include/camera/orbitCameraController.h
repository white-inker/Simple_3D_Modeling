//
// Created by Remilia on 25-12-18.
//

#ifndef ORBITCAMERACONTROLLER_H
#define ORBITCAMERACONTROLLER_H

#include <camera/cameraController.h>
#include <camera/orbitCamera.h>
#include <interaction/inputContext.h>

class OrbitCameraController : public CameraController
{
public:
    explicit OrbitCameraController(OrbitCamera& cam) : m_cam(std::move(cam)) {}

    float rotateSensitivity = 0.15f;
    float panSensitivity    = 0.002f;
    float zoomSensitivity   = 0.5f;

    int orbitMouseButton = GLFW_MOUSE_BUTTON_RIGHT;
    int panMouseButton   = GLFW_MOUSE_BUTTON_MIDDLE;

    void update(const InputContext& input, float dt, void* ptr) override;
    Camera& getCamera() override { return m_cam;}

private:
    OrbitCamera m_cam;
};


#endif //ORBITCAMERACONTROLLER_H
