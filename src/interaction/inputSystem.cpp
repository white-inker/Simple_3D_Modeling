//
// Created by Omari on 25-12-17.
//

#include "interaction/InputSystem.h"
#include "interaction/InputContext.h"
#include "utils/windowContext.h"

void InputSystem::Attach(GLFWwindow *window, WindowContext *context)
{
    glfwSetWindowUserPointer(window, context);

    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}

// ================= GLFW 回调 =================

void InputSystem::KeyCallback(GLFWwindow *window, int key, int, int action, int)
{
    const auto ctx = static_cast<WindowContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->input->OnKey(key, action);
}

void InputSystem::MouseButtonCallback(GLFWwindow *window, int button, int action, int)
{
    const auto ctx = static_cast<WindowContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->input->OnMouseButton(button, action);
}

void InputSystem::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    const auto ctx = static_cast<WindowContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->input->OnMouseMove(
            static_cast<float>(xpos),
            static_cast<float>(ypos)
        );
}

void InputSystem::ScrollCallback(GLFWwindow *window, double, double yoffset)
{
    const auto ctx = static_cast<WindowContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->input->OnScroll(static_cast<float>(yoffset));
}

void InputSystem::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);

    const auto ctx = static_cast<WindowContext *>(glfwGetWindowUserPointer(window));
    if (ctx && height > 0)
    {
        ctx->cameraController->getCamera().setAspect(width, height);
    }

}