#include "Game/Entities/Arrow.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "Game/TheGame.hpp"

//-----------------------------------------------------------------------------------
Arrow::Arrow(Entity* owner, uint16_t networkId) 
    : Entity()
    , m_speed(5.0f)
    , m_power(1.0f)
    , m_lifeSpan(2.0f)
    , m_owner(owner)
{
    m_networkId = networkId;
    m_sprite = new Sprite("Arrow", TheGame::WEAPON_LAYER);
    m_sprite->m_scale = Vector2(1.0f, 1.0f);

    m_sprite->m_position = m_owner->m_sprite->m_position;
    m_sprite->m_rotationDegrees = m_owner->m_sprite->m_rotationDegrees;
}

//-----------------------------------------------------------------------------------
Arrow::~Arrow()
{
}

//-----------------------------------------------------------------------------------
void Arrow::Update(float deltaSeconds)
{
    Entity::Update(deltaSeconds);
    if (m_age < m_lifeSpan)
    {
        Vector2 direction = Vector2::DegreesToDirection(-m_sprite->m_rotationDegrees, Vector2::ZERO_DEGREES_UP);
        Vector2 deltaVelocity = direction * m_speed * deltaSeconds;
        m_sprite->m_position += deltaVelocity;
    }
    else
    {
        m_isDead = true;
    }
}

//-----------------------------------------------------------------------------------
void Arrow::Render() const
{

}

//-----------------------------------------------------------------------------------
void Arrow::ResolveCollision(Entity* otherEntity)
{
    Entity::ResolveCollision(otherEntity);
    if (otherEntity != m_owner)
    {
        otherEntity->TakeDamage(m_power);
        this->m_isDead = true;
    }
}

