//
// Created by Remilia on 25-12-20.
//

#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include <core/application.h>
#include <geometry/primitiveFactory.h>
#include <camera/fpsCameraController.h>

#include "core/window.h"
#include "scene/scene.h"

class TestApplication final : public Application
{
public:
    explicit TestApplication(const std::string& name) : Application(name){}
    void initSceneEntity() override;
    void initWindowCamera(const WindowConfig &config, Window &window) override;
};

inline void TestApplication::initSceneEntity()
{
    PrimitiveDesc pDesc;
    pDesc.type = PrimitiveType::Cube;
    pDesc.size = {1.0, 1.0, 1.0};
    auto cubeModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.size = {20, 0.01, 20};
    auto floorModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Sphere;
    pDesc.radius = 1.0f;
    auto sphereModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Cylinder;
    pDesc.sides = 64;
    auto cylinderModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Cone;
    auto coneModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Prism;
    pDesc.sides = 7;
    auto prismModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Frustum;
    auto frustumModel = PrimitiveFactory::CreateModel(pDesc);

    auto pbrShader = assetManager->loadShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
    if (!pbrShader) LOG(ERROR) << "Shader load fail" ;
    auto material = Material();
    material.shader = pbrShader;

    auto& floorEntity = scene->createEntity("Floor");
    floorEntity.setModel(floorModel);
    floorEntity.setMaterial(material);
    floorEntity.getTransform().setPosition({0.0, -1.0, 0.0});

    auto& cubeEntity = scene->createEntity("Cube");
    cubeEntity.setModel(cubeModel);
    cubeEntity.setMaterial(material);

    auto& sphereEntity = scene->createEntity("Sphere");
    sphereEntity.setModel(sphereModel);
    sphereEntity.setMaterial(material);
    sphereEntity.getTransform().setPosition({0.0, 2.0, 0.0});

    auto& cylinderEntity = scene->createEntity("Cylinder");
    cylinderEntity.setModel(cylinderModel);
    cylinderEntity.setMaterial(material);
    cylinderEntity.getTransform().setPosition({2.0, 0.0, 0.0});

    auto& coneEntity = scene->createEntity("Cone");
    coneEntity.setModel(coneModel);
    coneEntity.setMaterial(material);
    coneEntity.getTransform().setPosition({0.0, 0.0, 2.0});

    auto& prismEntity = scene->createEntity("Prism");
    prismEntity.setModel(prismModel);
    prismEntity.setMaterial(material);
    prismEntity.getTransform().setPosition({-2.0, 0.0, 0.0});

    auto& frustumEntity = scene->createEntity("Frustum");
    frustumEntity.setModel(frustumModel);
    frustumEntity.setMaterial(material);
    frustumEntity.getTransform().setPosition({0.0, 0.0, -1.5});

    scene->getLighting().directional()->direction = {0.5, -1.0, -0.5};
    scene->getLighting().directional()->intensity = 2.0;
    scene->getLighting().directional()->color = {0.8, 0.8, 0.9};
    scene->getLighting().directional()->enableShadow = true;

    scene->getLighting().pointLights()[0]->position = {-10.0, 2.0, 0.0};
    scene->getLighting().pointLights()[1]->enabled = false;
}

inline void TestApplication::initWindowCamera(const WindowConfig &config, Window &window)
{
    // 默认 FPS 相机
    initWindowFPSCamera(config, window);
    // 你可以在下面设置相机参数
    dynamic_cast<FpsCameraController*>(window.cameraController.get())->enableCollision = true;
}



#endif //TESTAPPLICATION_H
