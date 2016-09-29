#pragma once
#include <vector>

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
    void OnPlayerAttack(const NetSender& from, NetMessage message);
    void OnPlayerDamaged(const NetSender& from, NetMessage message);

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Link* m_localPlayer;
    std::vector<Link*> m_players;
};