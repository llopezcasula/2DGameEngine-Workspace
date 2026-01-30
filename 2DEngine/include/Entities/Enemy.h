#pragma once

#include "Entities/Entity.h"
#include <glm/glm.hpp>

class Texture;

class Enemy : public Entity
{
public:
    Enemy(const glm::vec2& position, float speed = 100.0f);
    virtual ~Enemy() = default;

    void Update(float deltaTime) override;
    void Render() override;

    bool IsAlive() const { return m_Alive; }
    void Kill() { m_Alive = false; }

    void SetSpeed(float speed) { m_Speed = speed; }
    void SetColor(const glm::vec4& color) { m_Color = color; }
    void SetSize(const glm::vec2& size);
    void SetDirection(const glm::vec2& direction) { m_Direction = glm::normalize(direction); }

    int GetHealth() const { return m_Health; }
    void SetHealth(int health) { m_Health = health; }
    void TakeDamage(int damage);

    // Texture support
    void SetTexture(Texture* texture) { m_Texture = texture; }
    Texture* GetTexture() const { return m_Texture; }

    // Sprite sheet animation support
    void SetUseSpriteSheet(bool useSpriteSheet) { m_UseSpriteSheet = useSpriteSheet; }
    void SetAnimationFrame(int frame);  // 0 or 1
    int GetCurrentFrame() const { return m_CurrentFrame; }

    // For non-sprite-sheet animation (legacy support)
    void SetAnimationTextures(Texture* frame1, Texture* frame2);

protected:
    glm::vec2 m_Size;
    glm::vec2 m_Direction;
    glm::vec4 m_Color;
    float m_Speed;
    int m_Health;
    bool m_Alive;

    // Texture support
    Texture* m_Texture;          // Current texture (or sprite sheet)
    Texture* m_AnimFrame1;       // Animation frame 1 (for separate textures)
    Texture* m_AnimFrame2;       // Animation frame 2 (for separate textures)
    int m_CurrentFrame;          // Current animation frame (0 or 1)

    // Sprite sheet support
    bool m_UseSpriteSheet;       // Whether to use sprite sheet texture coordinates
    glm::vec4 m_TexCoords;       // Texture coordinates for sprite sheet (minU, minV, maxU, maxV)
};