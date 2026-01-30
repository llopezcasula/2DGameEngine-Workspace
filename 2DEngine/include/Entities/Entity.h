#pragma once

#include <glm/glm.hpp>
#include <string>
#include <memory>

class Collider;

// Base class for all game objects
class Entity
{
public:
    Entity(const std::string& name = "Entity");
    virtual ~Entity();

    // Called every frame
    virtual void Update(float deltaTime) {}

    // Called every fixed tick
    virtual void FixedUpdate(float fixedDeltaTime) {}

    // Called for rendering
    virtual void Render() {}

    // Collision callbacks
    virtual void OnCollisionEnter(Entity* other) {}
    virtual void OnCollisionStay(Entity* other) {}
    virtual void OnCollisionExit(Entity* other) {}

    // Transform
    void SetPosition(const glm::vec2& position) { m_Position = position; }
    const glm::vec2& GetPosition() const { return m_Position; }

    void SetRotation(float rotation) { m_Rotation = rotation; }
    float GetRotation() const { return m_Rotation; }

    void SetScale(const glm::vec2& scale) { m_Scale = scale; }
    const glm::vec2& GetScale() const { return m_Scale; }

    // Properties
    void SetActive(bool active) { m_Active = active; }
    bool IsActive() const { return m_Active; }

    void SetName(const std::string& name) { m_Name = name; }  // ← Add this!
    const std::string& GetName() const { return m_Name; }

    // Collider
    Collider* GetCollider() const { return m_Collider.get(); }
    void SetCollider(std::unique_ptr<Collider> collider);

protected:
    std::string m_Name;
    glm::vec2 m_Position;
    glm::vec2 m_Scale;
    float m_Rotation;
    bool m_Active;

    std::unique_ptr<Collider> m_Collider;
};