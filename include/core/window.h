//
// Created by Omari on 25-12-17.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <utils/common.h>
#include <interaction/inputContext.h>

class Window
{
public:
    GLFWwindow* window;
    InputContext input;

    explicit Window(GLFWwindow* window);
    virtual ~Window();
    Window(const Window& window) = delete;
    Window& operator=(const Window& window) = delete;
    Window(Window&& window) = default;

    void init();
    virtual void handleInput();
    virtual void renderFrame();
};

#endif //WINDOW_H
