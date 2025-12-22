//
// Created by Remilia on 25-12-20.
//

#ifndef TESTAPPLICATION_H
#define TESTAPPLICATION_H

#include <core/application.h>
#include <geometry/primitiveFactory.h>

#include <scene/scene.h>

class TestApplication final : public Application
{
public:
    explicit TestApplication(const std::string name) : Application(name){}
    void initSceneEntity() override;
};

inline void TestApplication::initSceneEntity()
{
    //auto chairModel = assetManager->loadModelOBJ("kenney_furniture-kit/Models/OBJ format/chair.obj");
    auto kitchenBlenderModel = assetManager->loadModelGLTF2("minimalistic_modern_bedroom/scene.gltf");
    auto pbrShader = assetManager->loadShader("shaders/pbr/pbr.vert", "shaders/pbr/pbr.frag");
    if (!pbrShader) LOG(ERROR) << "Shader load fail" ;
    auto material = Material();
    material.shader = pbrShader;
    //auto& chairEntity = scene->createEntity("Chair");
    //chairEntity.setModel(chairModel);
    //chairEntity.setMaterial(material);\
    //chairEntity.getTransform().setPosition({0.0f, 0.0f, -2.0f});

    auto& kitchenBlenderEntity = scene->createEntity("KitchenBlender");
    kitchenBlenderEntity.setModel(kitchenBlenderModel);
    kitchenBlenderEntity.getTransform().setPosition({0.0f, 0.0f, 0.0f});
}



#endif //TESTAPPLICATION_H
