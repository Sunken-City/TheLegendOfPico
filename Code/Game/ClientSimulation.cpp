#include "Game/ClientSimulation.hpp"
#include "Game/Entities/Link.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Input/InputMap.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "TheGame.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"
#include "Engine/Input/InputValues.hpp"
#include "Engine/Math/Vector2.hpp"

//-----------------------------------------------------------------------------------
ClientSimulation::ClientSimulation()
    : m_localPlayer(nullptr)
{
    m_players.reserve(8);
    for (unsigned int i = 0; i < 8; ++i)
    {
        m_players.push_back(nullptr);
    }
}

//-----------------------------------------------------------------------------------
ClientSimulation::~ClientSimulation()
{
    for (Entity* ent : m_entities)
    {
        delete ent;
    }
    m_entities.clear();
    m_players.clear();
}

//-----------------------------------------------------------------------------------
void ClientSimulation::Update(float deltaSeconds)
{
    if (m_localPlayer)
    {
        SpriteGameRenderer::instance->SetCameraPosition(m_localPlayer->m_sprite->m_position);
    }
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnUpdateFromHostReceived(const NetSender& from, NetMessage& message)
{
    if (from.connection)
    {
        uint8_t idx = from.connection->m_index;
        for (Link* networkedPlayer : m_players)
        {
            if (networkedPlayer)
            {
                message.Read<Vector2>(networkedPlayer->m_sprite->m_position);
                message.Read<Link::Facing>(networkedPlayer->m_facing);
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void ClientSimulation::SendNetClientUpdate(NetConnection* cp)
{
    NetMessage update(GameNetMessages::CLIENT_TO_HOST_UPDATE);
    InputMap& input = TheGame::instance->m_gameplayMapping;
    InputAxis* rightAxis = input.FindInputAxis("Right");
    InputAxis* upAxis = input.FindInputAxis("Up");
    Vector2 rightAxisValues(rightAxis->m_positiveValue->m_currentValue, rightAxis->m_negativeValue->m_currentValue);
    Vector2 upAxisValues(upAxis->m_positiveValue->m_currentValue, upAxis->m_negativeValue->m_currentValue);
    update.Write<Vector2>(rightAxisValues);
    update.Write<Vector2>(upAxisValues);
    cp->SendMessage(update);
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnConnectionJoined(NetConnection* cp)
{
    Link* player = new Link();
    player->m_netOwnerIndex = cp->m_index;
    m_players[cp->m_index] = player;
    m_entities.push_back(player);
    if (cp->m_index == NetSession::instance->GetMyConnectionIndex())
    {
        m_localPlayer = player;
    }
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnConnectionLeave(NetConnection* cp)
{
    uint8_t idx = cp->m_index;
    for (auto iter = m_players.begin(); iter != m_players.end(); ++iter)
    {
        Link* networkedPlayer = *iter;
        if (networkedPlayer && networkedPlayer->m_netOwnerIndex == idx)
        {
            auto entityItr = std::find(m_entities.begin(), m_entities.end(), networkedPlayer);
            m_entities.erase(entityItr);
            m_localPlayer = m_localPlayer == networkedPlayer ? nullptr : m_localPlayer;
            delete networkedPlayer;
            iter = m_players.erase(iter);
            break;
        }
    }
}
