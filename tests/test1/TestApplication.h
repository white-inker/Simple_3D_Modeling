//
// Created by Remilia on 25-12-20.
//

#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include <core/application.h>
#include <geometry/primitiveFactory.h>

#include "scene/scene.h"

class TestApplication final : public Application
{
public:
    explicit TestApplication(const std::string name) : Application(name){}
    void initSceneEntity() override;
};

inline void TestApplication::initSceneEntity()
{
    PrimitiveDesc pDesc;
    pDesc.type = PrimitiveType::Cube;
    pDesc.size = {1.0, 1.0, 1.0};
    auto cubeModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Sphere;
    pDesc.radius = 1.0f;
    auto sphereModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Cylinder;
    pDesc.sides = 64;
    auto cylinderModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Cone;
    auto coneModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Prism;
    pDesc.sides = 5;
    auto prismModel = PrimitiveFactory::CreateModel(pDesc);

    pDesc.type = PrimitiveType::Frustum;
    auto frustumModel = PrimitiveFactory::CreateModel(pDesc);

    auto pbrShader = assetManager->loadShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
    if (!pbrShader) LOG(ERROR) << "Shader load fail" ;
    auto material = Material();
    material.shader = pbrShader;

    auto& cubeEntity = scene->createEntity("Cube");
    cubeEntity.setModel(cubeModel);
    cubeEntity.setMaterial(material);

    auto& sphereEntity = scene->createEntity("Sphere");
    sphereEntity.setModel(sphereModel);
    sphereEntity.setMaterial(material);
    sphereEntity.getTransform().setPosition({-2.0, 0.0, 0.0});

    auto& cylinderEntity = scene->createEntity("Cylinder");
    cylinderEntity.setModel(cylinderModel);
    cylinderEntity.setMaterial(material);
    cylinderEntity.getTransform().setPosition({2.0, 0.0, 0.0});

    auto& coneEntity = scene->createEntity("Cone");
    coneEntity.setModel(coneModel);
    coneEntity.setMaterial(material);
    coneEntity.getTransform().setPosition({4.0, 0.0, 0.0});

    auto& prismEntity = scene->createEntity("Prism");
    prismEntity.setModel(prismModel);
    prismEntity.setMaterial(material);
    prismEntity.getTransform().setPosition({6.0, 0.0, 0.0});

    auto& frustumEntity = scene->createEntity("Frustum");
    frustumEntity.setModel(frustumModel);
    frustumEntity.setMaterial(material);
    frustumEntity.getTransform().setPosition({8.0, 0.0, 0.0});
}



#endif //TESTAPPLICATION_H
