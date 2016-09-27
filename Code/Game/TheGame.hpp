#pragma once

#include <vector>
#include "Engine/Math/Vector2.hpp"
#include "Engine/Net/UDPIP/NetMessage.hpp"
#include "Engine/Input/InputMap.hpp"

class Entity;
class Player;
class Ship;
class NetConnection;
struct NetSender;

//-----------------------------------------------------------------------------------
enum GameNetMessages
{
    GAME_UPDATE = NetMessage::CoreMessageTypes::NUM_MESSAGES,
};

//-----------------------------------------------------------------------------------
class TheGame
{
public:
    TheGame();
    ~TheGame();
    void OnConnectionJoined(NetConnection* cp);
    void OnConnectionLeave(NetConnection* cp);
    void OnNetTick(NetConnection* cp);
    void OnUpdateReceive(const NetSender& from, NetMessage& message);
    void Update(float deltaTime);
    void Render() const;
    void SpawnBullet(Entity* creator);
    void SpawnPickup(const Vector2& spawnPosition);

    static TheGame* instance;

    //CONSTANTS/////////////////////////////////////////////////////////////////////
    static unsigned int const BACKGROUND_LAYER = 0;
    static unsigned int const ITEM_LAYER = 8;
    static unsigned int const ENEMY_LAYER = 9;
    static unsigned int const PLAYER_LAYER = 10;
    static unsigned int const PLAYER_BULLET_LAYER = 11;
    static unsigned int const ENEMY_BULLET_LAYER = 12;
    static unsigned int const FOREGROUND_LAYER = 20;
    static unsigned int const UI_LAYER = 30;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Player*> m_players;
    Player* m_localPlayer;
    int m_debuggingControllerIndex;
    InputMap m_gameplayMapping;

private:
    TheGame& operator= (const TheGame& other) = delete;
    void CleanupGameOverState(unsigned int);
    void UpdateGameOver(float deltaSeconds);
    void RenderGameOver() const;
    void RegisterSprites();
    void InitializePlayingState();
    void UpdatePlaying(float deltaSeconds);
    void RenderPlaying() const;
    void InitializeGameOverState();
    void CleanupPlayingState(unsigned int);

    void InitializeMainMenuState();
    void InitializeKeyMappings();
    void CleanupMainMenuState(unsigned int);
    void UpdateMainMenu(float deltaSeconds);
    void RenderMainMenu() const;

    //MEMBER VARIABLES/////////////////////////////////////////////////////////////////////
    std::vector<Entity*> m_entities;
    std::vector<Entity*> m_newEntities;
};
