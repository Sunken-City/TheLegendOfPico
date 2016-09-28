#include "Game/HostSimulation.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/Link.hpp"
#include "Game/Entities/Pickup.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"
#include "Engine/Renderer/2D/Sprite.hpp"
#include "TheGame.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"

//-----------------------------------------------------------------------------------
HostSimulation::HostSimulation()
{
    InitializeKeyMappings();
    m_players.reserve(8);
    for (unsigned int i = 0; i < 8; ++i)
    {
        m_players.push_back(nullptr);
    }
}

//-----------------------------------------------------------------------------------
HostSimulation::~HostSimulation()
{
    UninitializeKeyMappings();
    for (Entity* ent : m_entities)
    {
        delete ent;
    }
    m_entities.clear();
    m_players.clear();
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnUpdateFromClientReceived(const NetSender& from, NetMessage& message)
{
    Vector2 rightValues;
    Vector2 upValues;
    message.Read<Vector2>(rightValues);
    message.Read<Vector2>(upValues);
    m_networkMapping.FindInputAxis("Right")->SetValue(rightValues.x, rightValues.y);
    m_networkMapping.FindInputAxis("Up")->SetValue(upValues.x, upValues.y);
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnConnectionJoined(NetConnection* cp)
{
    Link* player = new Link();
    player->m_netOwnerIndex = cp->m_index;
    m_players[cp->m_index] = player;
    m_entities.push_back(player);
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnConnectionLeave(NetConnection* cp)
{
    uint8_t idx = cp->m_index;
    for (auto iter = m_players.begin(); iter != m_players.end(); ++iter)
    {
        Link* networkedPlayer = *iter;
        if (networkedPlayer && networkedPlayer->m_netOwnerIndex == idx)
        {
            auto entityItr = std::find(m_entities.begin(), m_entities.end(), networkedPlayer);
            m_entities.erase(entityItr);
            delete networkedPlayer;
            iter = m_players.erase(iter);
            break;
        }
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::SendNetHostUpdate(NetConnection* cp)
{
    for (Link* link : m_players)
    {
        if (link)
        {
            NetMessage update(GameNetMessages::HOST_TO_CLIENT_UPDATE);
            update.Write<Vector2>(link->m_sprite->m_position);
            update.Write<Link::Facing>(link->m_facing);
            cp->SendMessage(update);
        }
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::Update(float deltaSeconds)
{
    UpdateEntities(deltaSeconds);
    AddNewEntities();
    CleanUpDeadEntities();
}

//-----------------------------------------------------------------------------------
void HostSimulation::UpdateEntities(float deltaSeconds)
{
    for (Entity* ent : m_entities)
    {
        ent->Update(deltaSeconds);
        for (Entity* other : m_entities)
        {
            if ((ent != other) && (ent->IsCollidingWith(other)))
            {
                ent->ResolveCollision(other);
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::AddNewEntities()
{
    for (Entity* ent : m_newEntities)
    {
        m_entities.push_back(ent);
    }
    m_newEntities.clear();
}

//-----------------------------------------------------------------------------------
void HostSimulation::CleanUpDeadEntities()
{
    for (auto iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        Entity* gameObject = *iter;
        if (gameObject->m_isDead)
        {
            delete gameObject;
            iter = m_entities.erase(iter);
        }
        if (iter == m_entities.end())
        {
            break;
        }
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::SpawnArrow(Entity* creator)
{
    //m_newEntities.push_back(new Bullet(creator));
}

//-----------------------------------------------------------------------------------
void HostSimulation::SpawnPickup(const Vector2& spawnPosition)
{
    m_newEntities.push_back(new Pickup(spawnPosition));
}

//-----------------------------------------------------------------------------------
void HostSimulation::InitializeKeyMappings()
{
    m_networkMapping.AddInputAxis("Up", new InputValue(&m_networkMapping), new InputValue(&m_networkMapping));
    m_networkMapping.AddInputAxis("Right", new InputValue(&m_networkMapping), new InputValue(&m_networkMapping));
}

//-----------------------------------------------------------------------------------
void HostSimulation::UninitializeKeyMappings()
{
    InputAxis* tempAxis = m_networkMapping.FindInputAxis("Up");
    delete tempAxis->m_positiveValue;
    delete tempAxis->m_negativeValue;

    tempAxis = m_networkMapping.FindInputAxis("Right");
    delete tempAxis->m_positiveValue;
    delete tempAxis->m_negativeValue;
}
