//
// Created by Omari on 25-12-17.
//

#include "interaction/InputSystem.h"
#include "interaction/InputContext.h"

void InputSystem::Attach(GLFWwindow *window, InputContext *context)
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
    auto *ctx = static_cast<InputContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->OnKey(key, action);
}

void InputSystem::MouseButtonCallback(GLFWwindow *window, int button, int action, int)
{
    auto *ctx = static_cast<InputContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->OnMouseButton(button, action);
}

void InputSystem::CursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    auto *ctx = static_cast<InputContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->OnMouseMove(
            static_cast<float>(xpos),
            static_cast<float>(ypos)
        );
}

void InputSystem::ScrollCallback(GLFWwindow *window, double, double yoffset)
{
    auto *ctx = static_cast<InputContext *>(
        glfwGetWindowUserPointer(window)
    );
    if (ctx)
        ctx->OnScroll(static_cast<float>(yoffset));
}

void InputSystem::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}