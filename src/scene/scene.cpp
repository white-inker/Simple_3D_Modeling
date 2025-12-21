//
// Created by Remilia on 25-12-18.
//
#include <scene/scene.h>
#include <stdexcept>

Entity& Scene::createEntity(const std::string& name)
{
    auto e = std::make_unique<Entity>(name);
    return addEntity(std::move(e));
}

Entity& Scene::addEntity(Entity::Ptr entity)
{
    if (!entity)
    {
        LOG(ERROR) << "Scene::addEntity got null entity";
        throw std::runtime_error("Scene::addEntity got null entity");
    }
    entities.emplace_back(std::move(entity));
    return *entities.back();
}

void Scene::removeAt(size_t index)
{
    if (index >= entities.size()) return;
    entities.erase(entities.begin() + static_cast<long>(index));
}

Entity* Scene::at(size_t index)
{
    if (index >= entities.size()) return nullptr;
    return entities[index].get();
}

const Entity* Scene::at(size_t index) const
{
    if (index >= entities.size()) return nullptr;
    return entities[index].get();
}
