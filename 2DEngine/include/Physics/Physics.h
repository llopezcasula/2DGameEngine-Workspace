#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>  // Add this include

class Collider;
class Entity;

// Simple physics/collision manager
class Physics
{
public:
    static void Init();
    static void Shutdown();

    // Register/unregister colliders
    static void RegisterCollider(Collider* collider);
    static void UnregisterCollider(Collider* collider);

    // Check collision for a specific collider
    static bool CheckCollision(const Collider* collider, Collider** outOther = nullptr);

    // Get all colliders colliding with this one
    static std::vector<Collider*> GetCollisions(const Collider* collider);

    // Raycast (check if a line intersects any collider)
    static bool Raycast(const glm::vec2& start, const glm::vec2& end,
                       Collider** outHit = nullptr, glm::vec2* outPoint = nullptr);

    // Debug rendering
    static void DebugRenderColliders();
    static void SetDebugDraw(bool enabled) { s_DebugDraw = enabled; }
    static bool IsDebugDrawEnabled() { return s_DebugDraw; }

private:
    static std::vector<Collider*> s_Colliders;
    static bool s_DebugDraw;
};