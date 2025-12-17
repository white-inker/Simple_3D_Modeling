//
// Created by Omari on 25-12-17.
//

#include <core/window.h>

#include "interaction/inputSystem.h"

Window::Window(GLFWwindow *window) : window(window) { }

Window::~Window()
{
    if (window)
        glfwDestroyWindow(window);
}

void Window::init()
{
    InputSystem::Attach(window, &input);
}

void Window::handleInput()
{
    LOG(ERROR) << "Window::handleInput hasn't been implemented yet!";
}

void Window::renderFrame()
{
    LOG(ERROR) << "Window::randerFrame hasn't been implemented yet!";
}


