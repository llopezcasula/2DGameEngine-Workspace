#include "Entities/Enemy.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"
#include "Physics/Collider.h"

Enemy::Enemy(const glm::vec2& position, float speed)
    : Entity("Enemy")
    , m_Size(40.0f, 40.0f)
    , m_Direction(0.0f, -1.0f) // Default: move downward
    , m_Color(1.0f, 0.0f, 0.0f, 1.0f) // Red by default
    , m_Speed(speed)
    , m_Health(1)
    , m_Alive(true)
    , m_Texture(nullptr)
    , m_AnimFrame1(nullptr)
    , m_AnimFrame2(nullptr)
    , m_CurrentFrame(0)
    , m_UseSpriteSheet(false)
    , m_TexCoords(0.0f, 0.0f, 1.0f, 1.0f)  // Default: full texture
{
    m_Position = position;

    // Add collider
    auto collider = std::make_unique<BoxCollider>(this, m_Size);
    collider->SetTrigger(true);
    SetCollider(std::move(collider));
}

void Enemy::SetSize(const glm::vec2& size)
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

void Enemy::TakeDamage(int damage)
{
    m_Health -= damage;
    if (m_Health <= 0)
    {
        m_Alive = false;
    }
}

void Enemy::Update(float deltaTime)
{
    if (!m_Alive) return;

    // Move in direction
    m_Position += m_Direction * m_Speed * deltaTime;

    // Kill if way off screen (adjust bounds as needed)
    if (m_Position.y < -500.0f || m_Position.y > 500.0f ||
        m_Position.x < -700.0f || m_Position.x > 700.0f)
    {
        m_Alive = false;
    }
}

void Enemy::Render()
{
    if (!m_Alive || !m_Active) return;

    if (m_Texture)
    {
        if (m_UseSpriteSheet)
        {
            // Render with sprite sheet texture coordinates
            Renderer::DrawQuadWithTexCoords(m_Position, m_Size, m_Texture, m_TexCoords, m_Color);
        }
        else
        {
            // Render with full texture (old method)
            Renderer::DrawQuad(m_Position, m_Size, m_Texture, m_Color);
        }
    }
    else
    {
        // Render solid color (fallback)
        Renderer::DrawQuad(m_Position, m_Size, m_Color);
    }
}

void Enemy::SetAnimationTextures(Texture* frame1, Texture* frame2)
{
    m_AnimFrame1 = frame1;
    m_AnimFrame2 = frame2;
    m_CurrentFrame = 0;
    m_UseSpriteSheet = false;

    // Set initial texture
    if (m_AnimFrame1)
    {
        m_Texture = m_AnimFrame1;
    }
}

void Enemy::SetAnimationFrame(int frame)
{
    m_CurrentFrame = frame;

    if (m_UseSpriteSheet)
    {
        // Update texture coordinates for sprite sheet
        // Assuming 2 frames side-by-side (horizontal sprite sheet)
        if (frame == 0)
        {
            // Left half of sprite sheet
            m_TexCoords = glm::vec4(0.0f, 0.0f, 0.5f, 1.0f);
        }
        else
        {
            // Right half of sprite sheet
            m_TexCoords = glm::vec4(0.5f, 0.0f, 1.0f, 1.0f);
        }
    }
    else
    {
        // Update the texture based on current frame (old method)
        if (m_CurrentFrame == 0 && m_AnimFrame1)
        {
            m_Texture = m_AnimFrame1;
        }
        else if (m_CurrentFrame == 1 && m_AnimFrame2)
        {
            m_Texture = m_AnimFrame2;
        }
    }
}