#pragma once
#include "Engine/Components/Transform3D.hpp"
#include "Engine/Math/Vector2.hpp"
#include <stdint.h>

class Sprite;

class Entity
{
public:
    Entity();
    virtual ~Entity();

    virtual void Update(float deltaSeconds);
    virtual void Render() const;
    virtual bool IsCollidingWith(Entity* otherEntity);
    virtual void ResolveCollision(Entity* otherEntity);
    virtual void TakeDamage(float m_power);
    virtual void ApplyClientUpdate();
    inline virtual bool IsPlayer() { return false; }
    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    uint16_t m_networkId;
    Sprite* m_sprite;
    Vector2 m_position;
    float m_rotationDegrees;
    float m_hp;
    float m_maxHp;
    float m_collisionRadius;
    float m_age;
    bool m_isDead;
};