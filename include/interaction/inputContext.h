//
// Created by Omari on 25-12-17.
//

#ifndef INPUTCONTEXT_H
#define INPUTCONTEXT_H

#include <utils/common.h>

class InputContext
{
public:
    InputContext();

    // ===== GLFW 事件入口=====
    void OnKey(int key, int action);
    void OnMouseButton(int button, int action);
    void OnMouseMove(float x, float y);
    void OnScroll(float offset);

    // ===== 每帧更新 =====
    void Update();

    // ===== 查询接口 =====
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;

    bool IsMouseButtonDown(int button) const;

    glm::vec2 GetMousePosition() const;
    glm::vec2 GetMouseDelta() const;
    float GetScrollOffset() const;

private:
    bool m_keys[GLFW_KEY_LAST + 1]{};
    bool m_keysPressed[GLFW_KEY_LAST + 1]{};
    bool m_keysReleased[GLFW_KEY_LAST + 1]{};

    bool m_mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1]{};

    glm::vec2 m_mousePos{0.0f};
    glm::vec2 m_lastMousePos{0.0f};
    glm::vec2 m_mouseDelta{0.0f};

    float m_scrollOffset{0.0f};
};

#endif //INPUTCONTEXT_H
