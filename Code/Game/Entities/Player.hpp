#pragma once
#include "Game/Entities/Entity.hpp"
#include <stdint.h>

class Player : public Entity
{
public:
    enum Facing
    {
        WEST,
        NORTH,
        EAST,
        SOUTH,
        NUM_DIRECTIONS
    };

    Player();
    ~Player();
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void ResolveCollision(Entity* otherEntity);
    Facing GetFacingFromInput(const Vector2& inputDirection);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint8_t m_netOwnerIndex;
    Facing m_facing;
    float m_speed;
    float m_power;
    float m_rateOfFire;
    float m_timeSinceLastShot;
};