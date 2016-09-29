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
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Renderer/2D/ParticleSystemDefinition.hpp"
#include "Engine/Audio/Audio.hpp"

//-----------------------------------------------------------------------------------
ClientSimulation::ClientSimulation()
    : m_localPlayer(nullptr)
{
    m_players.reserve(8);
    for (unsigned int i = 0; i < 8; ++i)
    {
        m_players.push_back(nullptr);
    }
    TheGame::instance->m_gameplayMapping.FindInputValue("Attack")->m_OnPress.RegisterMethod(this, &ClientSimulation::OnLocalPlayerAttackInput);
}

//-----------------------------------------------------------------------------------
ClientSimulation::~ClientSimulation()
{
    for (Link* link : m_players)
    {
        delete link;
    }
    m_players.clear();
}

//-----------------------------------------------------------------------------------
void ClientSimulation::Update(float deltaSeconds)
{
    if (m_localPlayer)
    {
        SpriteGameRenderer::instance->SetCameraPosition(m_localPlayer->m_position);
    }
    else
    {
#pragma todo("This is a workaround for an issue where the client gets a reliable inorder message before they've recieved their accept message. #helpC4")
        if (NetSession::instance->GetMyConnectionIndex() != NetSession::INVALID_CONNECTION_INDEX && m_players[NetSession::instance->GetMyConnectionIndex()] != nullptr)
        {
            m_localPlayer = m_players[NetSession::instance->GetMyConnectionIndex()];
        }
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
                message.Read<Vector2>(networkedPlayer->m_position);
                message.Read<Link::Facing>(networkedPlayer->m_facing);
                networkedPlayer->ApplyClientUpdate();
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

}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnConnectionLeave(NetConnection* cp)
{
//     uint8_t idx = cp->m_index;
//     for (auto iter = m_players.begin(); iter != m_players.end(); ++iter)
//     {
//         Link* networkedPlayer = *iter;
//         if (networkedPlayer && networkedPlayer->m_netOwnerIndex == idx)
//         {
//             auto entityItr = std::find(m_entities.begin(), m_entities.end(), networkedPlayer);
//             m_entities.erase(entityItr);
//             m_localPlayer = m_localPlayer == networkedPlayer ? nullptr : m_localPlayer;
//             delete networkedPlayer;
//             iter = m_players.erase(iter);
//             break;
//         }
//     }
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnPlayerCreate(const NetSender& from, NetMessage message)
{
    UNUSED(from);
    Link* player = new Link();
    unsigned int color = 0;

    //Read in link data
    message.Read<uint8_t>(player->m_netOwnerIndex);
    message.Read<unsigned int>(color);

    player->SetColor(color);
    m_players[player->m_netOwnerIndex] = player;
    if (player->m_netOwnerIndex == NetSession::instance->GetMyConnectionIndex())
    {
        m_localPlayer = player;
    }
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnPlayerDestroy(const NetSender& from, NetMessage message)
{
    uint8_t index = NetSession::INVALID_CONNECTION_INDEX;
    message.Read<uint8_t>(index);
    delete m_players[index];
    m_players[index] = nullptr;
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnLocalPlayerAttackInput(const InputValue*)
{
    bool isRequest = true;
    NetMessage attackMessage(GameNetMessages::PLAYER_ATTACK);
    attackMessage.Write<bool>(isRequest);
    NetSession::instance->m_hostConnection->SendMessage(attackMessage);
}

//-----------------------------------------------------------------------------------
void ClientSimulation::OnPlayerAttack(const NetSender& from, NetMessage message)
{
    static const SoundID swordSound1 = AudioSystem::instance->CreateOrGetSound("Data\\SFX\\Oracle_Sword_Slash1.wav");
    static const SoundID swordSound2 = AudioSystem::instance->CreateOrGetSound("Data\\SFX\\Oracle_Sword_Slash2.wav");
    static const SoundID swordSound3 = AudioSystem::instance->CreateOrGetSound("Data\\SFX\\Oracle_Sword_Slash3.wav");

    bool isRequest = true;
    uint8_t index = NetSession::INVALID_CONNECTION_INDEX;
    Vector2 swordPosition(0.0f);
    float swordRotation(0.0f);
    Link* attackingPlayer;
    message.Read<bool>(isRequest);
    message.Read<uint8_t>(index);
    message.Read<Vector2>(swordPosition);
    message.Read<float>(swordRotation);

    if (!isRequest)
    {
        ASSERT_OR_DIE(index < TheGame::MAX_PLAYERS, "Invalid index attached to attack message");
        attackingPlayer = m_players[index];

        ResourceDatabase::instance->GetParticleSystemResource("SwordAttack")->m_emitterDefinitions[0]->m_initialTintPerParticle = attackingPlayer->m_color;
        ParticleSystem::PlayOneShotParticleEffect("SwordAttack", TheGame::WEAPON_LAYER, swordPosition, swordRotation);

        switch (MathUtils::GetRandomIntFromZeroTo(3))
        {
        case 0:
            AudioSystem::instance->PlaySound(swordSound1);
        case 1:
            AudioSystem::instance->PlaySound(swordSound2);
        case 2:
            AudioSystem::instance->PlaySound(swordSound3);
        default:
            break;
        }
    }
}
