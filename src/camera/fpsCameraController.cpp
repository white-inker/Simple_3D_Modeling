//
// Created by Remilia on 25-12-18.
//

#include <camera/fpsCameraController.h>

#include <core/window.h>
#include <geometry/aabb.h>
#include <scene/scene.h>
#include <scene/entity.h>
#include <limits>

namespace
{
    glm::vec3 resolveCollision(const glm::vec3& desiredPos,
                               const glm::vec3& currentPos,
                               const Scene* scene,
                               float radius,
                               float offset)
    {
        if (!scene || radius <= 0.0f)
            return desiredPos;

        glm::vec3 correctedPos = desiredPos;

        for (const auto& entityPtr : scene->getEntities())
        {
            if (!entityPtr || !entityPtr->isRenderable()) continue;
            const auto& model = entityPtr->getModel();
            if (!model) continue;
            const auto modelMat = entityPtr->getTransform().modelMatrix();
            const auto worldAABB = transformAABB(model->bounds, modelMat);
            if (!worldAABB.valid) continue;

            // only check when within range to reduce cost
            if (distancePointAABB(worldAABB, currentPos) > radius * 4.0f)
                continue;

            if (sphereIntersectsAABB(worldAABB, correctedPos, radius))
            {
                // push out along minimal axis
                const glm::vec3 p = glm::clamp(correctedPos, worldAABB.min, worldAABB.max);
                const glm::vec3 delta = correctedPos - p;
                const float dist = glm::length(delta);
                if (dist < 1e-5f)
                {
                    // inside AABB center: push along closest face normal
                    float distances[6] = {
                        correctedPos.x - worldAABB.min.x,
                        worldAABB.max.x - correctedPos.x,
                        correctedPos.y - worldAABB.min.y,
                        worldAABB.max.y - correctedPos.y,
                        correctedPos.z - worldAABB.min.z,
                        worldAABB.max.z - correctedPos.z
                    };
                    float minDist = distances[0];
                    int axisIndex = 0;
                    for (int i = 1; i < 6; ++i)
                    {
                        if (distances[i] < minDist)
                        {
                            minDist = distances[i];
                            axisIndex = i;
                        }
                    }

                    const float push = radius + offset;
                    switch (axisIndex)
                    {
                        case 0: correctedPos.x = worldAABB.min.x - push; break; // left
                        case 1: correctedPos.x = worldAABB.max.x + push; break; // right
                        case 2: correctedPos.y = worldAABB.min.y - push; break; // bottom
                        case 3: correctedPos.y = worldAABB.max.y + push; break; // top
                        case 4: correctedPos.z = worldAABB.min.z - push; break; // back
                        case 5: correctedPos.z = worldAABB.max.z + push; break; // front
                        default: break;
                    }
                }
                else
                {
                    glm::vec3 dir = delta / dist;
                    correctedPos = p + dir * (radius + offset);
                }
            }
        }

        return correctedPos;
    }
}

void FpsCameraController::update(const InputContext& input, float dt, void* ptr)
{
    float speed = moveSpeed;
    if (input.IsKeyDown(GLFW_KEY_LEFT_SHIFT))
        speed *= fastMultiplier;
    if (input.IsKeyDown(GLFW_KEY_ESCAPE))
    {
        const auto win = static_cast<Window*>(ptr);
        glfwSetWindowShouldClose(win->window, true);
    }

    // 移动（WASD + QE）
    const auto fwd = m_cam.getForward();
    const auto right = m_cam.getRight();

    glm::vec3 curPos = m_cam.getPosition();
    glm::vec3 desiredPos = curPos;

    if (input.IsKeyDown(GLFW_KEY_W)) desiredPos += fwd * (speed * dt);
    if (input.IsKeyDown(GLFW_KEY_S)) desiredPos -= fwd * (speed * dt);
    if (input.IsKeyDown(GLFW_KEY_D)) desiredPos += right * (speed * dt);
    if (input.IsKeyDown(GLFW_KEY_A)) desiredPos -= right * (speed * dt);
    if (input.IsKeyDown(GLFW_KEY_E)) desiredPos += glm::vec3(0,1,0) * (speed * dt);
    if (input.IsKeyDown(GLFW_KEY_Q)) desiredPos += glm::vec3(0,-1,0) * (speed * dt);

    if (enableCollision)
    {
        const auto win = static_cast<Window*>(ptr);
        const Scene* scene = win ? win->scene : nullptr;
        desiredPos = resolveCollision(desiredPos, curPos, scene, collisionRadius, collisionOffset);
    }

    m_cam.setPosition(desiredPos);

    // 视角（按住右键旋转）
    const auto md = input.GetMouseDelta();
    m_cam.rotate(md.x * lookSensitivity, -md.y * lookSensitivity);

    // FOV
    const float scroll = input.GetScrollOffset();
    if (targetFovDeg < 0.0f) // 第一次进入时初始化
        targetFovDeg = m_cam.getFovYDegrees();

    if (scroll != 0.0f)
    {
        targetFovDeg -= scroll * fovScrollStepDeg;
        targetFovDeg = glm::clamp(targetFovDeg, fovMinDeg, fovMaxDeg);
    }

    if (smoothFov)
    {
        const float cur = m_cam.getFovYDegrees();
        const float alpha = 1.0f - std::exp(-fovSmoothSpeed * dt);
        const float newFov = cur + (targetFovDeg - cur) * alpha;

        m_cam.setPerspective(newFov, m_cam.getNear(), m_cam.getFar());
    }
    else
    {
        m_cam.setPerspective(targetFovDeg, m_cam.getNear(), m_cam.getFar());
    }
}
