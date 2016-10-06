#pragma once
#include <vector>
#include <map>

class Link;
class NetMessage;
class Entity;
class NetConnection;
class InputValue;
class Sprite;
struct NetSender;

class ClientSimulation
{
public:
    //CONSTRUCTORS/////////////////////////////////////////////////////////////////////
    ClientSimulation();
    ~ClientSimulation();

    //FUNCTIONS/////////////////////////////////////////////////////////////////////
    void Update(float deltaSeconds);
    void UpdateHearts(float hp);
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
    inline void ToggleTwah(const InputValue*) { m_isTwahMode = !m_isTwahMode; };

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    Link* m_localPlayer;
    unsigned int m_localPlayerColor;
    std::vector<Link*> m_players;
    std::map<uint16_t, Entity*> m_entities;
    Sprite* m_hearts[5];
    bool m_isTwahMode;
};