//
// Created by Remilia on 25-12-19.
//

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include <interaction/inputContext.h>
#include <camera/camera.h>

class CameraController
{
public:
    virtual ~CameraController() = default;

    virtual void update(const InputContext& input, float dt, void* ptr) = 0;

    virtual Camera& getCamera() = 0;
};

#endif //CAMERACONTROLLER_H
