#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    Camera(float width, float height);

    // Get the view-projection matrix
    const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    // Camera position in world space
    void SetPosition(const glm::vec2& position);
    const glm::vec2& GetPosition() const { return m_Position; }

    // Zoom level (1.0 = normal, 2.0 = zoomed in 2x, 0.5 = zoomed out 2x)
    void SetZoom(float zoom);
    float GetZoom() const { return m_Zoom; }

    // Update camera (call this if you change position or zoom)
    void RecalculateViewProjection();

    // Screen dimensions
    void SetViewportSize(float width, float height);

private:
    glm::mat4 m_ProjectionMatrix;
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ViewProjectionMatrix;

    glm::vec2 m_Position;
    float m_Zoom;

    float m_Width;
    float m_Height;
};