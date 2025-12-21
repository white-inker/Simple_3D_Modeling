//
// Created by Remilia on 25-12-18.
//

#include <camera/orbitCameraController.h>
#include <glm/glm.hpp>
#include <core/window.h>

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
}
