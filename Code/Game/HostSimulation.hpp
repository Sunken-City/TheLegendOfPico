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
    HostSimulation();
    ~HostSimulation();

    void SendNetHostUpdate(NetConnection* cp);
    void Update(float deltaSeconds);
    void UpdateEntities(float deltaSeconds);
    void AddNewEntities();
    void CleanUpDeadEntities();
    void SpawnArrow(Entity* creator);
    void SpawnPickup(const Vector2& spawnPosition);
    void InitializeKeyMappings();
    void UninitializeKeyMappings();
    void OnUpdateFromClientReceived(const NetSender& from, NetMessage& message);
    void OnConnectionJoined(NetConnection* cp);
    void OnConnectionLeave(NetConnection* cp);
    void OnPlayerDestroy(const NetSender& from, NetMessage& message);
    void OnPlayerCreate(const NetSender& from, NetMessage& message);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Link*> m_players;
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_newEntities;
    std::vector<InputMap> m_networkMappings;
};