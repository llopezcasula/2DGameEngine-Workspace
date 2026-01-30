#include "Entities/Obstacle.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"
#include "Physics/Collider.h"

Obstacle::Obstacle(const glm::vec2& position, const glm::vec2& size)
    : Entity("Obstacle")
    , m_Size(size)
    , m_Color(1.0f)
    , m_Texture(nullptr)
{
    m_Position = position;

    // Default obstacle collider (non-trigger)
    RebuildCollider(false);
}

void Obstacle::RebuildCollider(bool trigger)
{
    auto collider = std::make_unique<BoxCollider>(this, m_Size);
    collider->SetTrigger(trigger);
    SetCollider(std::move(collider));
}

void Obstacle::SetSize(const glm::vec2& size)
{
    m_Size = size;

    // Preserve trigger state if possible
    bool trigger = false;
    if (GetCollider())
    {
        trigger = GetCollider()->IsTrigger(); // if your Collider has this
    }

    RebuildCollider(trigger);
}

void Obstacle::SetTexture(const std::string& path)
{
    m_Texture = std::make_unique<Texture>(path);
}

void Obstacle::ClearTexture()
{
    m_Texture.reset();
}

Texture* Obstacle::GetTexture() const
{
    return m_Texture.get();
}

void Obstacle::Render()
{
    if (!m_Active) return;

    Quad quad;
    quad.position = m_Position;
    quad.size = m_Size;
    quad.color = m_Color;
    quad.rotation = m_Rotation;
    quad.texture = m_Texture.get();

    Renderer::DrawQuad(quad);
}
