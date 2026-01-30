#include "Physics/Physics.h"
#include "Physics/Collider.h"
#include <algorithm>

std::vector<Collider*> Physics::s_Colliders;
bool Physics::s_DebugDraw = false;

void Physics::Init()
{
    s_Colliders.clear();
    s_DebugDraw = false;
}

void Physics::Shutdown()
{
    s_Colliders.clear();
}

void Physics::RegisterCollider(Collider* collider)
{
    if (!collider) return;
    
    // Don't add duplicates
    auto it = std::find(s_Colliders.begin(), s_Colliders.end(), collider);
    if (it == s_Colliders.end())
    {
        s_Colliders.push_back(collider);
    }
}

void Physics::UnregisterCollider(Collider* collider)
{
    if (!collider) return;
    
    auto it = std::find(s_Colliders.begin(), s_Colliders.end(), collider);
    if (it != s_Colliders.end())
    {
        s_Colliders.erase(it);
    }
}

bool Physics::CheckCollision(const Collider* collider, Collider** outOther)
{
    if (!collider || !collider->IsEnabled()) return false;

    for (Collider* other : s_Colliders)
    {
        if (other == collider) continue;
        if (!other->IsEnabled()) continue;

        if (collider->CheckCollision(other))
        {
            if (outOther)
                *outOther = other;
            return true;
        }
    }

    return false;
}

std::vector<Collider*> Physics::GetCollisions(const Collider* collider)
{
    std::vector<Collider*> collisions;
    
    if (!collider || !collider->IsEnabled()) 
        return collisions;

    for (Collider* other : s_Colliders)
    {
        if (other == collider) continue;
        if (!other->IsEnabled()) continue;

        if (collider->CheckCollision(other))
        {
            collisions.push_back(other);
        }
    }

    return collisions;
}

bool Physics::Raycast(const glm::vec2& start, const glm::vec2& end, 
                     Collider** outHit, glm::vec2* outPoint)
{
    // TODO: Implement proper raycast
    // For now, just return false
    return false;
}

void Physics::DebugRenderColliders()
{
    if (!s_DebugDraw) return;

    for (Collider* collider : s_Colliders)
    {
        if (collider)
        {
            collider->DebugRender();
        }
    }
}