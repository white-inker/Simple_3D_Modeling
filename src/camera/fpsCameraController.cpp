//
// Created by Remilia on 25-12-18.
//

#include <camera/fpsCameraController.h>

#include <core/window.h>

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

    if (input.IsKeyDown(GLFW_KEY_W)) m_cam.moveWorld(fwd * (speed * dt));
    if (input.IsKeyDown(GLFW_KEY_S)) m_cam.moveWorld(-fwd * (speed * dt));
    if (input.IsKeyDown(GLFW_KEY_D)) m_cam.moveWorld(right * (speed * dt));
    if (input.IsKeyDown(GLFW_KEY_A)) m_cam.moveWorld(-right * (speed * dt));
    if (input.IsKeyDown(GLFW_KEY_E)) m_cam.moveWorld(glm::vec3(0,1,0) * (speed * dt));
    if (input.IsKeyDown(GLFW_KEY_Q)) m_cam.moveWorld(glm::vec3(0,-1,0) * (speed * dt));

    // 视角（按住右键旋转）
    //if (input.IsMouseButtonDown(lookMouseButton))
    //{
        const auto md = input.GetMouseDelta();
        m_cam.rotate(md.x * lookSensitivity, -md.y * lookSensitivity);
    //}

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
