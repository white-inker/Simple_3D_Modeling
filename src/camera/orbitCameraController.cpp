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

namespace
{
    AABB computeSceneBounds(const Scene* scene)
    {
        AABB bounds;
        bounds.reset();

        if (!scene) return bounds;

        for (const auto& entityPtr : scene->getEntities())
        {
            if (!entityPtr || !entityPtr->isRenderable()) continue;

            const auto& model = entityPtr->getModel();
            if (!model) continue;

            const glm::mat4 entityTransform = entityPtr->getTransform().modelMatrix();
            for (const auto& sm : model->submeshes)
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
        const float safeRadius = std::max(radius, 0.001f);
        const float aspect = std::max(cam.getAspect(), 0.01f);

        const float fovY = glm::radians(cam.getFovYDegrees());
        const float fovX = 2.0f * std::atan(std::tan(fovY * 0.5f) * aspect);

        const float distanceY = safeRadius / std::tan(fovY * 0.5f);
        const float distanceX = safeRadius / std::tan(fovX * 0.5f);

        const float baseDistance = std::max(distanceX, distanceY);
        return baseDistance + safeRadius * 0.2f; // small padding
    }
}

void OrbitCameraController::update(const InputContext& input, float /*dt*/, void* ptr)
{
    const glm::vec2 md = input.GetMouseDelta();
    if (input.IsKeyDown(GLFW_KEY_ESCAPE))
    {
        const auto win = static_cast<Window*>(ptr);
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

    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
    {
        const auto win = static_cast<Window*>(ptr);
        const Scene* scene = win ? win->scene : nullptr;
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
