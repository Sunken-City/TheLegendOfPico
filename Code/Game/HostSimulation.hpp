#pragma once
#include <vector>
#include "Engine\Input\InputMap.hpp"
#include "Engine\Net\UDPIP\NetSession.hpp"

class Entity;
class Link;
class NetConnection;
class NetMessage;
struct NetSender;

class HostSimulation
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    HostSimulation();
    ~HostSimulation();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void SendNetHostUpdate(NetConnection* cp);
    void Update(float deltaSeconds);
    void UpdateEntities(float deltaSeconds);
    void AddNewEntities();
    void CleanUpDeadEntities();
    void InitializeKeyMappings();
    void UninitializeKeyMappings();
    void OnConnectionJoined(NetConnection* cp);
    void OnConnectionLeave(NetConnection* cp);
    void BroadcastLinkCreation(uint8_t index, unsigned int playerColor);

    //These functions take a copy of the NetMessage intentionally, so that they can read the contents on their own
    void OnUpdateFromClientReceived(const NetSender& from, NetMessage& message);
    void OnPlayerDestroy(const NetSender& from, NetMessage message);
    void OnPlayerCreate(const NetSender& from, NetMessage message);
    void OnPlayerAttack(const NetSender& from, NetMessage message);
    void CheckForAndBroadcastDamage(Link* attackingPlayer, const Vector2& swordPosition);
    void OnPlayerFireBow(const NetSender& from, NetMessage& message);

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    const static int MAX_PLAYERS = NetSession::MAX_CONNECTIONS;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Link*> m_players;
    unsigned int m_playerColors[MAX_PLAYERS];
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_newEntities;
    std::vector<InputMap> m_networkMappings;
};