//
// Created by Omari on 25-12-17.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <utils/common.h>
#include <interaction/inputContext.h>
#include <camera/cameraController.h>
#include <camera/fpsCamera.h>
#include <camera/orbitCamera.h>
#include <utils/windowContext.h>

class Window
{
public:
    GLFWwindow* window;
    InputContext input;
    std::unique_ptr<CameraController> cameraController = nullptr;
    WindowContext windowContext{};
    bool screenshotRequested = false;

    explicit Window(GLFWwindow* window);
    virtual ~Window();
    Window(const Window& window) = delete;
    Window& operator=(const Window& window) = delete;
    Window(Window&& window) = default;

    void init();
    void initCameraController(FpsCamera& camera);
    void initCameraController(OrbitCamera& camera);
    void handleInput(float dt);
    void renderFrame();
    bool consumeScreenshotRequest();
};

#endif //WINDOW_H
