#include "UI/Button.h"
#include "Input/Input.h"
#include "Graphics/Renderer.h"
#include "Graphics/TextRenderer.h"
#include "Graphics/Camera.h"

Button::Button(const glm::vec2& position, const glm::vec2& size, const std::string& text)
    : m_Position(position)
    , m_Size(size)
    , m_Text(text)
    , m_ColorNormal(0.3f, 0.3f, 0.3f, 1.0f)
    , m_ColorHover(0.5f, 0.5f, 0.5f, 1.0f)
    , m_ColorPressed(0.2f, 0.2f, 0.2f, 1.0f)
    , m_IsHovered(false)
    , m_IsPressed(false)
    , m_WasPressed(false)
{
}

void Button::Update(const Camera& camera)
{
    glm::vec2 mouseWorld = Input::GetMouseWorldPosition(camera);
    m_IsHovered = IsPointInside(mouseWorld);

    bool mouseDown = Input::IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    m_IsPressed = m_IsHovered && mouseDown;

    // Detect click (mouse released while over button)
    if (m_WasPressed && !mouseDown && m_IsHovered && m_OnClick)
    {
        m_OnClick();
    }

    m_WasPressed = m_IsPressed;
}

void Button::Render()
{
    glm::vec4 color = m_ColorNormal;
    if (m_IsPressed)
        color = m_ColorPressed;
    else if (m_IsHovered)
        color = m_ColorHover;

    // Draw button background
    Renderer::DrawQuad(m_Position, m_Size, color);

    // Draw button border
    float borderThickness = 2.0f;
    glm::vec4 borderColor = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

    // Top border
    Renderer::DrawQuad(
        glm::vec2(m_Position.x, m_Position.y + m_Size.y/2),
        glm::vec2(m_Size.x, borderThickness),
        borderColor
    );
    // Bottom border
    Renderer::DrawQuad(
        glm::vec2(m_Position.x, m_Position.y - m_Size.y/2),
        glm::vec2(m_Size.x, borderThickness),
        borderColor
    );
    // Left border
    Renderer::DrawQuad(
        glm::vec2(m_Position.x - m_Size.x/2, m_Position.y),
        glm::vec2(borderThickness, m_Size.y),
        borderColor
    );
    // Right border
    Renderer::DrawQuad(
        glm::vec2(m_Position.x + m_Size.x/2, m_Position.y),
        glm::vec2(borderThickness, m_Size.y),
        borderColor
    );

    // Draw text
    if (!m_Text.empty())
    {
        float textScale = 1.5f;  // Increased scale for better readability
        float textWidth = TextRenderer::GetTextWidth(m_Text, textScale);
        float textHeight = 7.0f * 2.0f * textScale;  // Character height

        // Center text on button
        glm::vec2 textPos = m_Position;
        textPos.x -= textWidth / 2.0f;
        textPos.y += textHeight / 4.0f;  // Adjust for better vertical centering

        glm::vec4 textColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        TextRenderer::RenderText(m_Text, textPos, textScale, textColor);
    }
}

void Button::SetColors(const glm::vec4& normal, const glm::vec4& hover, const glm::vec4& pressed)
{
    m_ColorNormal = normal;
    m_ColorHover = hover;
    m_ColorPressed = pressed;
}

bool Button::IsPointInside(const glm::vec2& point) const
{
    float halfWidth = m_Size.x * 0.5f;
    float halfHeight = m_Size.y * 0.5f;

    return point.x >= m_Position.x - halfWidth &&
           point.x <= m_Position.x + halfWidth &&
           point.y >= m_Position.y - halfHeight &&
           point.y <= m_Position.y + halfHeight;
}