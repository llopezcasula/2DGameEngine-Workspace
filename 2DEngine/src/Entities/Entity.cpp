#include "Entities/Entity.h"
#include "Physics/Collider.h"
#include "Physics/Physics.h"

Entity::Entity(const std::string& name)
    : m_Name(name)
    , m_Position(0.0f, 0.0f)
    , m_Scale(1.0f, 1.0f)
    , m_Rotation(0.0f)
    , m_Active(true)
    , m_Collider(nullptr)
{
}

Entity::~Entity()
{
    if (m_Collider)
    {
        Physics::UnregisterCollider(m_Collider.get());
    }
}

void Entity::SetCollider(std::unique_ptr<Collider> collider)
{
    // Unregister old collider
    if (m_Collider)
    {
        Physics::UnregisterCollider(m_Collider.get());
    }

    // Set new collider
    m_Collider = std::move(collider);

    // Register new collider
    if (m_Collider)
    {
        Physics::RegisterCollider(m_Collider.get());
    }
}