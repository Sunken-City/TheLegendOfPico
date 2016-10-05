#pragma once
#include "Game/Entities/Entity.hpp"
#include "Engine/Math/Vector2.hpp"
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
    inline virtual bool IsPlayer() { return true; };

    void UpdateSpriteFromFacing();
    float CalculateSwordRotationDegrees();
    Vector2 CalculateSwordPosition();
    Facing GetFacingFromInput(const Vector2& inputDirection);
    void SetColor(unsigned int color);
    void ApplyClientUpdate();
    void ApplyDamageEffect();

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const float HURT_FLASH_DURATION_SECONDS = 0.5f;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint8_t m_netOwnerIndex;
    Facing m_facing;
    float m_speed;
    float m_rateOfFire;
    float m_timeSinceLastHurt;
    RGBA m_color;
    double m_lastHurtTime;
};