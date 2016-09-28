#include "Game/Entities/Link.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Game/TheGame.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Game/HostSimulation.hpp"

//-----------------------------------------------------------------------------------
Link::Link()
    : Entity()
    , m_netOwnerIndex(0)
    , m_facing(Facing::SOUTH)
    , m_speed(0.0f)
    , m_power(0.0f)
    , m_rateOfFire(0.0f)
    , m_timeSinceLastShot(0.0f)
    , m_color(RGBA::GetRandom())
{
    m_isDead = false;
    m_maxHp = 10.0f;
    m_hp = 10.0f;
    m_sprite = new Sprite("pDown", TheGame::PLAYER_LAYER);
    m_sprite->m_scale = Vector2(1.0f, 1.0f);
    m_sprite->m_tintColor = m_color;
    m_speed = 1.0f;
    m_rateOfFire = 0.5f;
}

//-----------------------------------------------------------------------------------
Link::~Link()
{

}

//-----------------------------------------------------------------------------------
void Link::Update(float deltaSeconds)
{
    ASSERT_OR_DIE(TheGame::instance->m_host, "Update for the player should not be called on the clients.");
    Entity::Update(deltaSeconds);
    m_timeSinceLastShot += deltaSeconds;
    float adjustedSpeed = m_speed / 20.0f;

    InputMap& input = TheGame::instance->m_host->m_networkMapping;
    Vector2 inputDirection = input.GetVector2("Right", "Up");
    Vector2 attemptedPosition = m_sprite->m_position + inputDirection * adjustedSpeed;

    //TODO: Bounds check
    m_sprite->m_position = attemptedPosition;

    m_facing = GetFacingFromInput(inputDirection);

    switch (m_facing)
    {
    case Link::WEST:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pRight");
        break;
    case Link::NORTH:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pUp");
        break;
    case Link::EAST:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pLeft");
        break;
    case Link::SOUTH:
        m_sprite->m_spriteResource = ResourceDatabase::instance->GetSpriteResource("pDown");
        break;
    default:
        break;
    }
}

//-----------------------------------------------------------------------------------
void Link::Render() const
{

}

//-----------------------------------------------------------------------------------
void Link::ResolveCollision(Entity* otherEntity)
{
    Entity::ResolveCollision(otherEntity);
}

//-----------------------------------------------------------------------------------
Link::Facing Link::GetFacingFromInput(const Vector2& inputDirection)
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

