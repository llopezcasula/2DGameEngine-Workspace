#pragma once

#include "Entities/Entity.h"
#include <glm/glm.hpp>

class Bullet : public Entity
{
public:
    Bullet(const glm::vec2& position, const glm::vec2& direction, float speed = 500.0f);
    virtual ~Bullet() = default;

    void Update(float deltaTime) override;
    void Render() override;
    
    bool IsAlive() const { return m_Alive; }
    void Kill() { m_Alive = false; }
    
    void SetLifetime(float lifetime) { m_MaxLifetime = lifetime; }
    void SetColor(const glm::vec4& color) { m_Color = color; }
    void SetSize(const glm::vec2& size);

protected:
    glm::vec2 m_Direction;
    glm::vec2 m_Size;
    glm::vec4 m_Color;
    float m_Speed;
    float m_Lifetime;
    float m_MaxLifetime;
    bool m_Alive;
};