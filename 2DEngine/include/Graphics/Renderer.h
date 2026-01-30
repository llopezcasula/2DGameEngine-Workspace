#pragma once

#include <glm/glm.hpp>
#include <memory>

class Shader;
class Texture;

// Represents a simple quad (rectangle) that can be drawn
struct Quad
{
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
    float rotation;
    Texture* texture;

    Quad(const glm::vec2& pos = glm::vec2(0.0f),
         const glm::vec2& sz  = glm::vec2(1.0f),
         const glm::vec4& col = glm::vec4(1.0f),
         float rot = 0.0f,
         Texture* tex = nullptr)
        : position(pos), size(sz), color(col), rotation(rot), texture(tex) {}
};

class Renderer
{
public:
    static void Init();
    static void Shutdown();

    // Set the camera/view matrix
    static void BeginScene(const glm::mat4& viewProjection);
    static void EndScene();

    // Draw quads
    static void DrawQuad(const Quad& quad);

    static void DrawQuad(const glm::vec2& position,
                         const glm::vec2& size,
                         const glm::vec4& color = glm::vec4(1.0f));

    static void DrawQuad(const glm::vec2& position,
                         const glm::vec2& size,
                         Texture* texture,
                         const glm::vec4& tint = glm::vec4(1.0f));

    // Draw quad with custom texture coordinates (for sprite sheets)
    static void DrawQuadWithTexCoords(const glm::vec2& position,
                                      const glm::vec2& size,
                                      Texture* texture,
                                      const glm::vec4& texCoords,  // (minU, minV, maxU, maxV)
                                      const glm::vec4& tint = glm::vec4(1.0f));

    // Clear the screen
    static void Clear(const glm::vec4& color = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f));

private:
    struct RendererData;
    static std::unique_ptr<RendererData> s_Data;
};