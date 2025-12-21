//
// Created by Remilia on 25-12-19.
//

#ifndef WINDOWCONTEXT_H
#define WINDOWCONTEXT_H

#include <camera/cameraController.h>
#include <interaction/inputContext.h>

struct WindowContext
{
    InputContext* input;
    CameraController* cameraController;
};

#endif //WINDOWCONTEXT_H
