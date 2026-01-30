#include "UI/Menu.h"
#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"

Menu::Menu()
    : m_Visible(false)
{
}

void Menu::Update(const Camera& camera)
{
    if (!m_Visible) return;

    // Update button positions to follow camera
    glm::vec2 cameraPos = camera.GetPosition();
    for (size_t i = 0; i < m_Buttons.size(); i++)
    {
        glm::vec2 worldPos = cameraPos + m_ButtonRelativePositions[i];
        m_Buttons[i]->SetPosition(worldPos);
    }

    // Update button states
    for (auto& button : m_Buttons)
    {
        button->Update(camera);
    }
}

void Menu::Render(const Camera& camera)
{
    if (!m_Visible) return;

    // Draw semi-transparent background overlay centered on camera
    glm::vec2 cameraPos = camera.GetPosition();
    Renderer::DrawQuad(
        cameraPos,
        glm::vec2(10000.0f, 10000.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 0.7f)
    );

    // Draw all buttons
    for (auto& button : m_Buttons)
    {
        button->Render();
    }
}

Button* Menu::AddButton(const std::string& text, const glm::vec2& relativePosition, const glm::vec2& size)
{
    auto button = std::make_unique<Button>(relativePosition, size, text);
    Button* ptr = button.get();
    m_Buttons.push_back(std::move(button));
    m_ButtonRelativePositions.push_back(relativePosition);
    return ptr;
}