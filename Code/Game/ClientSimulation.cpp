#include "Game/ClientSimulation.hpp"
#include "Game/Entities/Link.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Input/InputMap.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "TheGame.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"

//-----------------------------------------------------------------------------------
ClientSimulation::ClientSimulation()
    : m_localPlayer(new Link())
{
    m_players.reserve(8);
    for (Link* player : m_players)
    {
        player = nullptr;
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
    Vector2 inputDirection = input.GetVector2("Right", "Up");
    update.Write<Vector2>(inputDirection);
    cp->SendMessage(update);
}
