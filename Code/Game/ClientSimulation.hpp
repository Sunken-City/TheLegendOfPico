#pragma once
#include <vector>
#include <map>

class Link;
class NetMessage;
class Entity;
class NetConnection;
class InputValue;
struct NetSender;

class ClientSimulation
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    ClientSimulation();
    ~ClientSimulation();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void OnUpdateFromHostReceived(const NetSender& from, NetMessage& message);
    void SendNetClientUpdate(NetConnection* cp);
    void OnPlayerCreate(const NetSender& from, NetMessage message);
    void OnPlayerDestroy(const NetSender& from, NetMessage message);
    void OnLocalPlayerAttackInput(const InputValue* attackInput);
    void OnLocalPlayerFireBowInput(const InputValue* bowInput);
    void OnLocalPlayerRespawnInput(const InputValue* respawnInput);
    void OnPlayerAttack(const NetSender& from, NetMessage message);
    void OnPlayerDamaged(const NetSender& from, NetMessage message);
    void OnPlayerFireBow(const NetSender& from, NetMessage& message);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Link* m_localPlayer;
    std::vector<Link*> m_players;
    std::map<uint16_t, Entity*> m_entities;
};