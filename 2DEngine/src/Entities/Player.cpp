#define GLM_ENABLE_EXPERIMENTAL

#include "Entities/Player.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"
#include "Input/Input.h"
#include <glm/gtx/norm.hpp>

Player::Player()
    : Entity("Player")
    , m_Velocity(0.0f, 0.0f)
    , m_MaxSpeed(600.0f)
    , m_Acceleration(3000.0f)
    , m_Deceleration(3000.0f)
    , m_Color(0.2f, 0.8f, 0.3f, 1.0f)
    , m_Size(50.0f, 50.0f)
    , m_Texture(nullptr)
{
}

Player::~Player()
{
}

void Player::SetTexture(const std::string& path)
{
    m_Texture = std::make_unique<Texture>(path);
}

void Player::Update(float deltaTime)
{
    // Get input direction
    glm::vec2 inputDir(0.0f);

    if (Input::IsKeyPressed(KEY_UP) || Input::IsKeyPressed(KEY_W))
        inputDir.y += 1.0f;
    if (Input::IsKeyPressed(KEY_DOWN) || Input::IsKeyPressed(KEY_S))
        inputDir.y -= 1.0f;
    if (Input::IsKeyPressed(KEY_LEFT) || Input::IsKeyPressed(KEY_A))
        inputDir.x -= 1.0f;
    if (Input::IsKeyPressed(KEY_RIGHT) || Input::IsKeyPressed(KEY_D))
        inputDir.x += 1.0f;

    // Normalize input direction
    float inputLength2 = glm::length2(inputDir);
    if (inputLength2 > 0.0f)
    {
        inputDir = glm::normalize(inputDir);
    }

    // Apply acceleration or deceleration
    if (inputLength2 > 0.0f)
    {
        m_Velocity += inputDir * m_Acceleration * deltaTime;
    }
    else
    {
        float currentSpeed = glm::length(m_Velocity);
        if (currentSpeed > 0.0f)
        {
            float decelerationAmount = m_Deceleration * deltaTime;

            if (decelerationAmount >= currentSpeed)
            {
                m_Velocity = glm::vec2(0.0f);
            }
            else
            {
                glm::vec2 velocityDir = m_Velocity / currentSpeed;
                m_Velocity -= velocityDir * decelerationAmount;
            }
        }
    }

    // Clamp velocity to max speed
    float currentSpeed = glm::length(m_Velocity);
    if (currentSpeed > m_MaxSpeed)
    {
        m_Velocity = (m_Velocity / currentSpeed) * m_MaxSpeed;
    }

    // Apply velocity to position
    m_Position += m_Velocity * deltaTime;
}


void Player::Render()
{
    if (!m_Active) return;

    Quad quad;
    quad.position = m_Position;
    quad.size = m_Size;
    quad.color = m_Color;  // ← Make sure this line exists!
    quad.rotation = m_Rotation;
    quad.texture = m_Texture.get();

    Renderer::DrawQuad(quad);
}