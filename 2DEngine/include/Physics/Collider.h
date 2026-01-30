#pragma once

#include <glm/glm.hpp>

// Forward declaration
class Entity;

enum class ColliderType
{
    Box,    // AABB (Axis-Aligned Bounding Box)
    Circle
};

// Base collider class
class Collider
{
public:
    Collider(Entity* owner, ColliderType type);
    virtual ~Collider() = default;

    // Check collision with another collider
    bool CheckCollision(const Collider* other) const;

    // Get the collider's world position (entity position + offset)
    glm::vec2 GetWorldPosition() const;

    // Setters
    void SetOffset(const glm::vec2& offset) { m_Offset = offset; }
    void SetEnabled(bool enabled) { m_Enabled = enabled; }
    void SetTrigger(bool isTrigger) { m_IsTrigger = isTrigger; }

    // Getters
    const glm::vec2& GetOffset() const { return m_Offset; }
    ColliderType GetType() const { return m_Type; }
    Entity* GetOwner() const { return m_Owner; }
    bool IsEnabled() const { return m_Enabled; }
    bool IsTrigger() const { return m_IsTrigger; }

    // Debug rendering
    virtual void DebugRender() const = 0;

protected:
    Entity* m_Owner;
    ColliderType m_Type;
    glm::vec2 m_Offset;  // Offset from entity position
    bool m_Enabled;
    bool m_IsTrigger;    // If true, doesn't block movement but still detects collision
};

// Box collider (AABB)
class BoxCollider : public Collider
{
public:
    BoxCollider(Entity* owner, const glm::vec2& size);

    void SetSize(const glm::vec2& size) { m_Size = size; }
    const glm::vec2& GetSize() const { return m_Size; }

    // Get min/max bounds in world space
    glm::vec2 GetMin() const;
    glm::vec2 GetMax() const;

    void DebugRender() const override;

private:
    glm::vec2 m_Size;
};

// Circle collider
class CircleCollider : public Collider
{
public:
    CircleCollider(Entity* owner, float radius);

    void SetRadius(float radius) { m_Radius = radius; }
    float GetRadius() const { return m_Radius; }

    void DebugRender() const override;

private:
    float m_Radius;
};