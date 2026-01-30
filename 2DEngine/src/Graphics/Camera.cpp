#include "Graphics/Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float width, float height)
    : m_Position(0.0f, 0.0f), m_Zoom(1.0f), m_Width(width), m_Height(height)
{
    RecalculateViewProjection();
}

void Camera::SetPosition(const glm::vec2& position)
{
    m_Position = position;
    RecalculateViewProjection();
}

void Camera::SetZoom(float zoom)
{
    // Prevent zoom from going to 0 or negative
    m_Zoom = glm::max(zoom, 0.01f);
    RecalculateViewProjection();
}

void Camera::SetViewportSize(float width, float height)
{
    m_Width = width;
    m_Height = height;
    RecalculateViewProjection();
}

void Camera::RecalculateViewProjection()
{
    // Orthographic projection for 2D
    // This creates a coordinate system where (0,0) is the center of the screen
    // and the edges are at ±(width/2), ±(height/2) divided by zoom
    
    float left = -m_Width / (2.0f * m_Zoom);
    float right = m_Width / (2.0f * m_Zoom);
    float bottom = -m_Height / (2.0f * m_Zoom);
    float top = m_Height / (2.0f * m_Zoom);

    m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);

    // View matrix: moves the world opposite to camera position
    m_ViewMatrix = glm::mat4(1.0f);
    m_ViewMatrix = glm::translate(m_ViewMatrix, glm::vec3(-m_Position.x, -m_Position.y, 0.0f));

    // Combined matrix (order matters: projection * view)
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}