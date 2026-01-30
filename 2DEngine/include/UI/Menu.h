#pragma once

#include <vector>
#include <memory>
#include "UI/Button.h"

class Camera;

class Menu
{
public:
    Menu();

    void Update(const Camera& camera);
    void Render(const Camera& camera);  // Added camera parameter

    void Show() { m_Visible = true; }
    void Hide() { m_Visible = false; }
    void Toggle() { m_Visible = !m_Visible; }
    bool IsVisible() const { return m_Visible; }

    // Add buttons to menu (positions are relative to menu center)
    Button* AddButton(const std::string& text, const glm::vec2& relativePosition, const glm::vec2& size);

private:
    bool m_Visible;
    std::vector<std::unique_ptr<Button>> m_Buttons;
    std::vector<glm::vec2> m_ButtonRelativePositions;  // Store original relative positions
};