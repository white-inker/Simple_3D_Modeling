//
// Created by Remilia on 25-12-18.
//

#include <camera/orbitCameraController.h>
#include <glm/glm.hpp>
#include <core/window.h>
#include <algorithm>
#include <cmath>
#include <geometry/aabb.h>
#include <scene/entity.h>
#include <scene/scene.h>

AABB computeSceneBounds(const Scene *scene)
{
    AABB bounds;
    bounds.reset();

    if (!scene) return bounds;

    for (const auto &entityPtr: scene->getEntities())
    {
        if (!entityPtr || !entityPtr->isRenderable()) continue;

        const auto &model = entityPtr->getModel();
        if (!model) continue;

        const glm::mat4 entityTransform = entityPtr->getTransform().modelMatrix();
        for (const auto &sm: model->submeshes)
        {
            if (!sm.localBounds.valid) continue;
            const glm::mat4 worldMat = entityTransform * sm.local;
            bounds.include(transformAABB(sm.localBounds, worldMat));
        }
    }

    return bounds;
}

float computeFitDistance(const OrbitCamera& cam, float radius)
{
    float r = std::max(radius, 1e-3f);

    const float fill = 0.7f;
    const float padding = 0.05f;

    float aspect = std::max(cam.getAspect(), 1e-3f);

    float fovY = glm::radians(cam.getFovYDegrees());
    float halfY = 0.5f * fovY;
    float halfX = std::atan(std::tan(halfY) * aspect);

    float dY = r / std::tan(halfY);
    float dX = r / std::tan(halfX);

    float d = std::max(dX, dY);

    d = d * fill;
    d += r * padding;

    return d;
}

void OrbitCameraController::update(const InputContext &input, float /*dt*/, void *ptr)
{
    const glm::vec2 md = input.GetMouseDelta();
    if (input.IsKeyDown(GLFW_KEY_ESCAPE))
    {
        const auto win = static_cast<Window *>(ptr);
        glfwSetWindowShouldClose(win->window, true);
    }

    if (input.IsMouseButtonDown(orbitMouseButton))
    {
        m_cam.orbit(md.x * rotateSensitivity, -md.y * rotateSensitivity);
    }

    if (input.IsMouseButtonDown(panMouseButton))
    {
        const float scale = m_cam.getDistance();
        m_cam.pan(md.x * panSensitivity * scale, -md.y * panSensitivity * scale);
    }

    const float scroll = input.GetScrollOffset();
    if (scroll != 0.0f)
    {
        m_cam.zoom(scroll * zoomSensitivity);
    }

    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        const auto win = static_cast<Window *>(ptr);
        const Scene *scene = win ? win->scene : nullptr;
        const AABB bounds = computeSceneBounds(scene);

        if (bounds.valid)
        {
            const glm::vec3 center = (bounds.min + bounds.max) * 0.5f;
            const float radius = 0.5f * glm::length(bounds.max - bounds.min);

            m_cam.setTarget(center);
            m_cam.setDistance(computeFitDistance(m_cam, radius));
        }
    }
}
