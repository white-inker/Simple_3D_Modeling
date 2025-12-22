//
// Created by Remilia on 25-12-18.
//

#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <string>
#include <utils/common.h>

#include <scene/transform.h>
#include <assets/model.h>
#include <graphics/material.h>

class Entity
{
public:
    using Ptr = std::unique_ptr<Entity>;

    explicit Entity(std::string name = "");
    ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    Entity(Entity&&) noexcept = default;
    Entity& operator=(Entity&&) noexcept = default;

    // --- identity ---
    const std::string& getName() const { return name; }
    void setName(const std::string& n) { name = n; }

    bool isVisible() const { return visible; }
    void setVisible(bool v) { visible = v; }

    // --- components ---
    Transform& getTransform() { return transform; }
    const Transform& getTransform() const { return transform; }

    Material& getMaterial() { return material; }
    const Material& getMaterial() const { return material; }

    void setModel(std::shared_ptr<Model> model) { this->model = std::move(model); }
    const std::shared_ptr<Model>& getModel() const { return model; }

    void setMaterial(const Material& m) { material = m; }
    void setTransform(Transform& t) { transform = std::move(t);}

    bool isRenderable() const
    {
        return visible && model && !model->submeshes.empty() && (material.isValid() || model->hasMaterial());
    }

private:
    std::string name;
    bool visible = true;

    Transform transform;
    Material material;

    std::shared_ptr<Model> model;
};

#endif //ENTITY_H
