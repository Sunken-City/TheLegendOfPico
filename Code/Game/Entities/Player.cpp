#include "Game/Entities/Player.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"

//-----------------------------------------------------------------------------------
Player::Player()
    : Entity()
    , m_netOwnerIndex(0)
    , m_facing(Facing::SOUTH)
    , m_speed(0.0f)
    , m_power(0.0f)
    , m_rateOfFire(0.0f)
    , m_timeSinceLastShot(0.0f)
{
    m_isDead = false;
    m_maxHp = 10.0f;
    m_hp = 10.0f;
    m_sprite = new Sprite("pDown", TheGame::PLAYER_LAYER);
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
    Entity::Update(deltaSeconds);
    m_timeSinceLastShot += deltaSeconds;
    float adjustedSpeed = m_speed / 20.0f;

    InputMap& input = TheGame::instance->m_gameplayMapping;
    Vector2 inputDirection = input.GetVector2("Right", "Up");
    Vector2 attemptedPosition = m_sprite->m_position + inputDirection * adjustedSpeed;

    m_facing = GetFacingFromInput(inputDirection);
    switch (m_facing)
    {
    case Player::WEST:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pRight");
        break;
    case Player::NORTH:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pUp");
        break;
    case Player::EAST:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pLeft");
        break;
    case Player::SOUTH:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pDown");
        break;
    default:
        break;
    }
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
    Entity::ResolveCollision(otherEntity);
}

//-----------------------------------------------------------------------------------
Player::Facing Player::GetFacingFromInput(const Vector2& inputDirection)
{
    if (inputDirection.CalculateMagnitude() < 0.01f)
    {
        return m_facing;
    }
    float rightward = inputDirection.Dot(Vector2::UNIT_X);
    float leftward = inputDirection.Dot(-Vector2::UNIT_X);
    float upward = inputDirection.Dot(Vector2::UNIT_Y);
    float downward  = inputDirection.Dot(-Vector2::UNIT_Y);

    Facing bestDirection = Facing::SOUTH;
    float bestDifference = 1.0f - downward;
    if (1.0f - upward < bestDifference)
    {
        bestDifference = 1.0f - upward;
        bestDirection = Facing::NORTH;
    }
    if (1.0f - rightward < bestDifference)
    {
        bestDifference = 1.0f - rightward;
        bestDirection = Facing::WEST;
    }
    if (1.0f - leftward < bestDifference)
    {
        bestDifference = 1.0f - leftward;
        bestDirection = Facing::EAST;
    }
    return bestDirection;
}

