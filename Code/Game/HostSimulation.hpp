#pragma once
#include <vector>
#include "Engine\Input\InputMap.hpp"

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
    void OnUpdateFromClientReceived(const NetSender& from, NetMessage& message);
    void OnConnectionJoined(NetConnection* cp);
    void OnConnectionLeave(NetConnection* cp);

    //These functions take a copy of the NetMessage intentionally, so that they can read the contents on their own
    void OnPlayerDestroy(const NetSender& from, NetMessage message);
    void OnPlayerCreate(const NetSender& from, NetMessage message);
    void OnPlayerAttack(const NetSender& from, NetMessage message);
    void CheckForAndBroadcastDamage(Link* attackingPlayer, const Vector2& swordPosition);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Link*> m_players;
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_newEntities;
    std::vector<InputMap> m_networkMappings;
};