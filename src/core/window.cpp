//
// Created by Omari on 25-12-17.
//

#include <core/window.h>

#include "camera/fpsCameraController.h"
#include "camera/orbitCameraController.h"
#include "interaction/inputSystem.h"

Window::Window(GLFWwindow *window) : window(window) { }

Window::~Window()
{
    if (window)
        glfwDestroyWindow(window);
}

void Window::init()
{
    windowContext.cameraController = cameraController.get();
    windowContext.input = &input;
    InputSystem::Attach(window, &windowContext);
}

void Window::initCameraController(FpsCamera &camera)
{
    cameraController = std::make_unique<FpsCameraController>(camera);
}

void Window::initCameraController(OrbitCamera &camera)
{
    cameraController = std::make_unique<OrbitCameraController>(camera);
}

void Window::handleInput(float dt)
{
    // 处理相机的参数
    //LOG(ERROR) << "Window::handleInput hasn't been implemented yet!";
    cameraController->update(input, dt, this);
    input.Update();
}

void Window::renderFrame()
{
    //LOG(ERROR) << "Window::randerFrame hasn't been implemented yet!";
}


