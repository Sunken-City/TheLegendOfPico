#pragma once
#include "Game/Entities/Entity.hpp"
#include <stdint.h>

class Arrow : public Entity
{
public:
    Arrow(Entity* Owner, uint16_t networkId);
    virtual ~Arrow();

    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual void ResolveCollision(Entity* otherEntity);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    float m_speed;
    float m_power;
    float m_lifeSpan;
    uint16_t m_networkId;
    Entity* m_owner;
};