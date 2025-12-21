//
// Created by Omari on 25-12-17.
//

#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include <utils/common.h>

class WindowContext;

class InputSystem
{
public:
    static void Attach(GLFWwindow* window, WindowContext* context);

private:
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

#endif //INPUTSYSTEM_H
