#include "Entities/Bullet.h"
#include "Graphics/Renderer.h"
#include "Physics/Collider.h"

Bullet::Bullet(const glm::vec2& position, const glm::vec2& direction, float speed)
    : Entity("Bullet")
    , m_Direction(glm::normalize(direction))
    , m_Size(8.0f, 20.0f)
    , m_Color(1.0f, 1.0f, 0.0f, 1.0f) // Yellow by default
    , m_Speed(speed)
    , m_Lifetime(0.0f)
    , m_MaxLifetime(3.0f)
    , m_Alive(true)
{
    m_Position = position;
    
    // Add collider
    auto collider = std::make_unique<BoxCollider>(this, m_Size);
    collider->SetTrigger(true);
    SetCollider(std::move(collider));
}

void Bullet::SetSize(const glm::vec2& size)
{
    m_Size = size;
    
    // Update collider
    if (m_Collider)
    {
        auto* boxCollider = dynamic_cast<BoxCollider*>(m_Collider.get());
        if (boxCollider)
        {
            boxCollider->SetSize(size);
        }
    }
}

void Bullet::Update(float deltaTime)
{
    if (!m_Alive) return;
    
    m_Position += m_Direction * m_Speed * deltaTime;
    
    m_Lifetime += deltaTime;
    if (m_Lifetime > m_MaxLifetime)
    {
        m_Alive = false;
    }
}

void Bullet::Render()
{
    if (!m_Alive || !m_Active) return;
    
    Renderer::DrawQuad(m_Position, m_Size, m_Color);
}