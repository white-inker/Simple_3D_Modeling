//
// Created by Remilia on 25-12-20.
//

#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include <core/application.h>
#include <geometry/primitiveFactory.h>
#include <camera/fpsCameraController.h>

#include <scene/scene.h>

#include "core/window.h"

class TestApplication final : public Application
{
public:
    explicit TestApplication(const std::string name) : Application(name){}
    void initSceneEntity() override;
    void initWindowCamera(const WindowConfig &config, Window &window) override;
};

inline void TestApplication::initSceneEntity()
{
    //auto chairModel = assetManager->loadModelOBJ("kenney_furniture-kit/Models/OBJ format/chair.obj");
    auto backroomModel = assetManager->loadModelGLTF2("backroom/scene.gltf");
    auto pbrShader = assetManager->loadShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
    if (!pbrShader) LOG(ERROR) << "Shader load fail" ;
    auto material = Material();
    material.shader = pbrShader;
    //auto& chairEntity = scene->createEntity("Chair");
    //chairEntity.setModel(chairModel);
    //chairEntity.setMaterial(material);\
    //chairEntity.getTransform().setPosition({0.0f, 0.0f, -2.0f});

    auto& backroomEntity = scene->createEntity("Maze");
    backroomEntity.setModel(backroomModel);
    backroomEntity.getTransform().setPosition({0.0f, 0.0f, 0.0f});

    scene->getLighting().directional()->enabled = false;

    scene->getLighting().pointLights()[0]->enabled = false;
    scene->getLighting().pointLights()[1]->enabled = false;
}

inline void TestApplication::initWindowCamera(const WindowConfig &config, Window &window)
{
    initWindowFPSCamera(config, window);
    // 你可以在下面设置相机参数
    dynamic_cast<FpsCameraController*>(window.cameraController.get())->enableCollision = false;
}




#endif //TESTAPPLICATION_H
