#include "Physics/Collider.h"
#include "Entities/Entity.h"
#include "Graphics/Renderer.h"
#include <cmath>

// ============================================
// Base Collider
// ============================================

Collider::Collider(Entity* owner, ColliderType type)
    : m_Owner(owner)
    , m_Type(type)
    , m_Offset(0.0f, 0.0f)
    , m_Enabled(true)
    , m_IsTrigger(false)
{
}

glm::vec2 Collider::GetWorldPosition() const
{
    if (!m_Owner) return m_Offset;
    return m_Owner->GetPosition() + m_Offset;
}

// Forward declarations for collision checks
bool CheckBoxBox(const BoxCollider* a, const BoxCollider* b);
bool CheckCircleCircle(const CircleCollider* a, const CircleCollider* b);
bool CheckBoxCircle(const BoxCollider* box, const CircleCollider* circle);

bool Collider::CheckCollision(const Collider* other) const
{
    if (!m_Enabled || !other->IsEnabled()) return false;
    if (!m_Owner || !other->GetOwner()) return false;

    // Box vs Box
    if (m_Type == ColliderType::Box && other->GetType() == ColliderType::Box)
    {
        return CheckBoxBox(static_cast<const BoxCollider*>(this), 
                          static_cast<const BoxCollider*>(other));
    }
    
    // Circle vs Circle
    if (m_Type == ColliderType::Circle && other->GetType() == ColliderType::Circle)
    {
        return CheckCircleCircle(static_cast<const CircleCollider*>(this), 
                                static_cast<const CircleCollider*>(other));
    }
    
    // Box vs Circle
    if (m_Type == ColliderType::Box && other->GetType() == ColliderType::Circle)
    {
        return CheckBoxCircle(static_cast<const BoxCollider*>(this), 
                             static_cast<const CircleCollider*>(other));
    }
    
    // Circle vs Box
    if (m_Type == ColliderType::Circle && other->GetType() == ColliderType::Box)
    {
        return CheckBoxCircle(static_cast<const BoxCollider*>(other), 
                             static_cast<const CircleCollider*>(this));
    }

    return false;
}

// ============================================
// Box Collider
// ============================================

BoxCollider::BoxCollider(Entity* owner, const glm::vec2& size)
    : Collider(owner, ColliderType::Box)
    , m_Size(size)
{
}

glm::vec2 BoxCollider::GetMin() const
{
    glm::vec2 pos = GetWorldPosition();
    return pos - m_Size * 0.5f;
}

glm::vec2 BoxCollider::GetMax() const
{
    glm::vec2 pos = GetWorldPosition();
    return pos + m_Size * 0.5f;
}

void BoxCollider::DebugRender() const
{
    if (!m_Enabled) return;

    glm::vec4 color = m_IsTrigger ? 
        glm::vec4(0.0f, 1.0f, 0.0f, 0.3f) :  // Green for triggers
        glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);   // Red for solid colliders

    Renderer::DrawQuad(GetWorldPosition(), m_Size, color);
}

// ============================================
// Circle Collider
// ============================================

CircleCollider::CircleCollider(Entity* owner, float radius)
    : Collider(owner, ColliderType::Circle)
    , m_Radius(radius)
{
}

void CircleCollider::DebugRender() const
{
    if (!m_Enabled) return;

    glm::vec4 color = m_IsTrigger ? 
        glm::vec4(0.0f, 1.0f, 0.0f, 0.3f) :
        glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);

    // Approximate circle with a square for now
    // TODO: Implement proper circle rendering
    float diameter = m_Radius * 2.0f;
    Renderer::DrawQuad(GetWorldPosition(), glm::vec2(diameter, diameter), color);
}

// ============================================
// Collision Detection Functions
// ============================================

bool CheckBoxBox(const BoxCollider* a, const BoxCollider* b)
{
    glm::vec2 aMin = a->GetMin();
    glm::vec2 aMax = a->GetMax();
    glm::vec2 bMin = b->GetMin();
    glm::vec2 bMax = b->GetMax();

    // AABB collision detection
    return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
           (aMin.y <= bMax.y && aMax.y >= bMin.y);
}

bool CheckCircleCircle(const CircleCollider* a, const CircleCollider* b)
{
    glm::vec2 aPos = a->GetWorldPosition();
    glm::vec2 bPos = b->GetWorldPosition();
    
    float distance = glm::length(aPos - bPos);
    float radiusSum = a->GetRadius() + b->GetRadius();
    
    return distance <= radiusSum;
}

bool CheckBoxCircle(const BoxCollider* box, const CircleCollider* circle)
{
    glm::vec2 boxPos = box->GetWorldPosition();
    glm::vec2 circlePos = circle->GetWorldPosition();
    glm::vec2 halfSize = box->GetSize() * 0.5f;

    // Find the closest point on the box to the circle center
    glm::vec2 closest;
    closest.x = glm::clamp(circlePos.x, boxPos.x - halfSize.x, boxPos.x + halfSize.x);
    closest.y = glm::clamp(circlePos.y, boxPos.y - halfSize.y, boxPos.y + halfSize.y);

    // Calculate distance between circle center and closest point
    float distance = glm::length(circlePos - closest);
    
    return distance <= circle->GetRadius();
}