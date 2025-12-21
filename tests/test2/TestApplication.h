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

}



#endif //TESTAPPLICATION_H
