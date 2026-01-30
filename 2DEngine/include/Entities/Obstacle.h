#pragma once

#include "Entities/Entity.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

class Texture;

class Obstacle : public Entity
{
public:
    Obstacle(const glm::vec2& position, const glm::vec2& size);
    ~Obstacle() override = default;

    void Render() override;

    void SetColor(const glm::vec4& color) { m_Color = color; }
    const glm::vec4& GetColor() const { return m_Color; }

    const glm::vec2& GetSize() const { return m_Size; }

    // IMPORTANT: updates collider too
    void SetSize(const glm::vec2& size);

    // If you want a standalone textured obstacle
    void SetTexture(const std::string& path);
    void ClearTexture();

    Texture* GetTexture() const;

private:
    void RebuildCollider(bool trigger);

private:
    glm::vec2 m_Size;
    glm::vec4 m_Color;

    // Optional owned texture
    std::unique_ptr<Texture> m_Texture;
};
