#pragma once

#include <glm/glm.hpp>
#include <string>
#include <functional>

class Camera;

class Button
{
public:
    using CallbackFn = std::function<void()>;

    Button(const glm::vec2& position, const glm::vec2& size, const std::string& text = "");

    void Update(const Camera& camera);
    void Render();

    // Set callback when button is clicked
    void SetOnClick(CallbackFn callback) { m_OnClick = callback; }

    // Appearance
    void SetColors(const glm::vec4& normal, const glm::vec4& hover, const glm::vec4& pressed);
    void SetText(const std::string& text) { m_Text = text; }

    // Transform
    void SetPosition(const glm::vec2& pos) { m_Position = pos; }
    void SetSize(const glm::vec2& size) { m_Size = size; }

    glm::vec2 GetPosition() const { return m_Position; }
    glm::vec2 GetSize() const { return m_Size; }

    bool IsHovered() const { return m_IsHovered; }
    bool IsPressed() const { return m_IsPressed; }

private:
    bool IsPointInside(const glm::vec2& point) const;

    glm::vec2 m_Position;
    glm::vec2 m_Size;
    std::string m_Text;

    glm::vec4 m_ColorNormal;
    glm::vec4 m_ColorHover;
    glm::vec4 m_ColorPressed;

    bool m_IsHovered;
    bool m_IsPressed;
    bool m_WasPressed;

    CallbackFn m_OnClick;
};