#include "Game/Entities/Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Game/TheGame.hpp"

//-----------------------------------------------------------------------------------
Player::Player()
    : Ship()
    , m_netOwnerIndex(0)
{
    m_isDead = false;
    m_maxHp = 99999999.0f;
    m_hp = 99999999.0f;
    m_sprite = new Sprite("Player", TheGame::PLAYER_LAYER);
    m_sprite->m_scale = Vector2(1.0f, 1.0f);
    m_speed = 1.0f;
    m_rateOfFire = 0.5f;
}

//-----------------------------------------------------------------------------------
Player::~Player()
{

}

//-----------------------------------------------------------------------------------
void Player::Update(float deltaSeconds)
{
    if (this != TheGame::instance->m_localPlayer)
    {
        return;
    }
    Ship::Update(deltaSeconds);
    float adjustedSpeed = m_speed / 60.0f;
    InputMap& input = TheGame::instance->m_gameplayMapping;
    Vector2 attemptedPosition = m_sprite->m_position + input.GetVector2("Right", "Up") * adjustedSpeed;
    //TODO: Bounds check
    m_sprite->m_position = attemptedPosition;
}

//-----------------------------------------------------------------------------------
void Player::Render() const
{

}

//-----------------------------------------------------------------------------------
void Player::ResolveCollision(Entity* otherEntity)
{
    Ship::ResolveCollision(otherEntity);
}

