//
// Created by Remilia on 25-12-18.
//

#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>
#include <scene/entity.h>
#include <scene/lighting.h>

class Scene
{
public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Entity& createEntity(const std::string& name);

    Entity& addEntity(Entity::Ptr entity);

    void removeAt(size_t index);

    size_t size() const { return entities.size(); }
    Entity* at(size_t index);
    const Entity* at(size_t index) const;

    const std::vector<Entity::Ptr>& getEntities() const { return entities; }
    std::vector<std::unique_ptr<Entity>>& getEntities() { return entities; }

    Lighting& getLighting() { return lighting; }
    const Lighting& getLighting() const { return lighting; }

private:
    std::vector<Entity::Ptr> entities;
    Lighting lighting;
};

#endif //SCENE_H
