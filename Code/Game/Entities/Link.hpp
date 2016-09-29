#pragma once
#include "Game/Entities/Entity.hpp"
#include "Engine/Renderer/RGBA.hpp"
#include <stdint.h>

class Link : public Entity
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

    Link(const RGBA& color = RGBA::WHITE);
    ~Link();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void ResolveCollision(Entity* otherEntity);

    void UpdateSpriteFromFacing();
    Facing GetFacingFromInput(const Vector2& inputDirection);
    void SetColor(unsigned int color);
    void ApplyClientUpdate();

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint8_t m_netOwnerIndex;
    Facing m_facing;
    float m_speed;
    float m_power;
    float m_rateOfFire;
    float m_timeSinceLastShot;
    RGBA m_color;
};