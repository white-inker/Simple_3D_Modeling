//
// Created by Omari on 25-12-17.
//

#include "interaction/InputContext.h"
#include <cstring>

InputContext::InputContext()
{
    std::memset(m_keys, 0, sizeof(m_keys));
    std::memset(m_keysPressed, 0, sizeof(m_keysPressed));
    std::memset(m_keysReleased, 0, sizeof(m_keysReleased));
    std::memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
}

// ================= GLFW 事件 =================

void InputContext::OnKey(int key, int action)
{
    if (key < 0) return;

    if (action == GLFW_PRESS)
    {
        m_keys[key] = true;
        m_keysPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        m_keys[key] = false;
        m_keysReleased[key] = true;
    }
}

void InputContext::OnMouseButton(int button, int action)
{
    if (button < 0) return;

    if (action == GLFW_PRESS)
        m_mouseButtons[button] = true;
    else if (action == GLFW_RELEASE)
        m_mouseButtons[button] = false;
}

void InputContext::OnMouseMove(float x, float y)
{
    m_mousePos = { x, y };
}

void InputContext::OnScroll(float offset)
{
    m_scrollOffset += offset;
}

// ================= 每帧更新 =================

void InputContext::Update()
{
    std::memset(m_keysPressed, 0, sizeof(m_keysPressed));
    std::memset(m_keysReleased, 0, sizeof(m_keysReleased));

    m_mouseDelta = m_mousePos - m_lastMousePos;
    m_lastMousePos = m_mousePos;

    m_scrollOffset = 0.0f;
}

// ================= 查询接口 =================

bool InputContext::IsKeyDown(int key) const
{
    return m_keys[key];
}

bool InputContext::IsKeyPressed(int key) const
{
    return m_keysPressed[key];
}

bool InputContext::IsKeyReleased(int key) const
{
    return m_keysReleased[key];
}

bool InputContext::IsMouseButtonDown(int button) const
{
    return m_mouseButtons[button];
}

glm::vec2 InputContext::GetMousePosition() const
{
    return m_mousePos;
}

glm::vec2 InputContext::GetMouseDelta() const
{
    return m_mouseDelta;
}

float InputContext::GetScrollOffset() const
{
    return m_scrollOffset;
}