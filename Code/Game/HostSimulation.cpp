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
    m_networkMappings[from.connection->m_index].FindInputAxis("Right")->SetValue(rightValues.x, rightValues.y);
    m_networkMappings[from.connection->m_index].FindInputAxis("Up")->SetValue(upValues.x, upValues.y);
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnConnectionJoined(NetConnection* cp)
{
    RGBA newLinkColor = RGBA::GetRandom();
    //Let everyone know about the guy we just created (Including ourselves!).
    for (NetConnection* conn : NetSession::instance->m_allConnections)
    {
        if (conn)
        {
            NetMessage message(GameNetMessages::PLAYER_CREATE);
            message.Write<uint8_t>(cp->m_index);
            message.Write<unsigned int>(newLinkColor.ToUnsignedInt());
            conn->SendMessage(message);
        }
    }

    //Bring the client up to speed.
    for (Link* link : m_players)
    {
        if (link)
        {
            NetMessage message(GameNetMessages::PLAYER_CREATE);
            message.Write<uint8_t>(link->m_netOwnerIndex);
            message.Write<unsigned int>(link->m_color.ToUnsignedInt());
            cp->SendMessage(message);
        }
    }
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
void HostSimulation::OnPlayerDestroy(const NetSender& from, NetMessage message)
{
    throw std::logic_error("The method or operation is not implemented.");
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnPlayerCreate(const NetSender& from, NetMessage message)
{
    Link* player = new Link();
    unsigned int color = 0;

    //Read the link data
    message.Read<uint8_t>(player->m_netOwnerIndex);
    message.Read<unsigned int>(color);

    player->SetColor(color);
    player->m_sprite->Disable();
    m_players[player->m_netOwnerIndex] = player;
    m_entities.push_back(player);
}

//-----------------------------------------------------------------------------------
void HostSimulation::OnPlayerAttack(const NetSender& from, NetMessage message)
{
    bool isRequest = false;
    uint8_t index = from.connection->m_index;
    message.Read<bool>(isRequest);

    Vector2 swordPosition = m_players[index]->CalculateSwordPosition();
    float swordRotation = m_players[index]->CalculateSwordRotationDegrees();

    if (isRequest)
    {
        for (NetConnection* conn : NetSession::instance->m_allConnections)
        {
            if (conn)
            {
                bool isRequest = false;
                NetMessage attackMessage(GameNetMessages::PLAYER_ATTACK);
                attackMessage.Write<bool>(isRequest);
                attackMessage.Write<uint8_t>(index);
                attackMessage.Write<Vector2>(swordPosition);
                attackMessage.Write<float>(swordRotation);
                NetSession::instance->m_hostConnection->SendMessage(attackMessage);
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::SendNetHostUpdate(NetConnection* cp)
{
    NetMessage update(GameNetMessages::HOST_TO_CLIENT_UPDATE);
    for (Link* link : m_players)
    {
        if (link)
        {
            update.Write<Vector2>(link->m_position);
            update.Write<Link::Facing>(link->m_facing);
        }
    }
    cp->SendMessage(update);
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
    //m_newEntities.push_back(new Pickup(spawnPosition));
}

//-----------------------------------------------------------------------------------
void HostSimulation::InitializeKeyMappings()
{
    m_networkMappings.resize(8);
    for (unsigned int i = 0; i < 8; ++i)
    {
        m_networkMappings[i].AddInputAxis("Up", new InputValue(&m_networkMappings[i]), new InputValue(&m_networkMappings[i]));
        m_networkMappings[i].AddInputAxis("Right", new InputValue(&m_networkMappings[i]), new InputValue(&m_networkMappings[i]));
    }
}

//-----------------------------------------------------------------------------------
void HostSimulation::UninitializeKeyMappings()
{
    for (unsigned int i = 0; i < 8; ++i)
    {
        InputAxis* tempAxis = m_networkMappings[i].FindInputAxis("Up");
        delete tempAxis->m_positiveValue;
        delete tempAxis->m_negativeValue;

        tempAxis = m_networkMappings[i].FindInputAxis("Right");
        delete tempAxis->m_positiveValue;
        delete tempAxis->m_negativeValue;
    }
}
