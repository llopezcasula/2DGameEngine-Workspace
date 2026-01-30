#pragma once

#include "Entities/Entity.h"
#include "Graphics/Texture.h"
#include <glm/glm.hpp>
#include <memory>

class Player : public Entity
{
public:
    Player();
    ~Player();

    void Update(float deltaTime) override;
    void Render() override;

    // Player specific
    void SetMaxSpeed(float speed) { m_MaxSpeed = speed; }
    float GetMaxSpeed() const { return m_MaxSpeed; }

    void SetAcceleration(float accel) { m_Acceleration = accel; }
    float GetAcceleration() const { return m_Acceleration; }

    void SetDeceleration(float decel) { m_Deceleration = decel; }
    float GetDeceleration() const { return m_Deceleration; }

    void SetColor(const glm::vec4& color) { m_Color = color; }
    const glm::vec4& GetColor() const { return m_Color; }

    void SetSize(const glm::vec2& size) { m_Size = size; }
    const glm::vec2& GetSize() const { return m_Size; }

    const glm::vec2& GetVelocity() const { return m_Velocity; }

    // Texture support
    void SetTexture(const std::string& path);
    void ClearTexture() { m_Texture.reset(); }

private:
    // Physics
    glm::vec2 m_Velocity;
    float m_MaxSpeed;
    float m_Acceleration;
    float m_Deceleration;

    // Appearance
    glm::vec4 m_Color;
    glm::vec2 m_Size;
    std::unique_ptr<Texture> m_Texture;
};