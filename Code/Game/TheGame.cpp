#include "Game/TheGame.hpp"
#include "Game/StateMachine.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Input/Console.hpp"
#include "Engine/Input/Logging.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixStack4x4.hpp"
#include "Engine/Renderer/MeshRenderer.hpp"
#include "Engine/Renderer/MeshBuilder.hpp"
#include "Engine/Renderer/Framebuffer.hpp"
#include "Engine/Renderer/2D/SpriteGameRenderer.hpp"
#include "Engine/Renderer/2D/ResourceDatabase.hpp"
#include "Engine/Input/XInputController.hpp"
#include "Engine/TextRendering/TextBox.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Input/XMLUtils.hpp"
#include "Game/Entities/Entity.hpp"
#include "Game/Entities/Player.hpp"
#include "Game/Entities/Bullet.hpp"
#include "Game/Entities/Ship.hpp"
#include "Entities/ItemBox.hpp"
#include "Entities/Grunt.hpp"
#include "Entities/Pickup.hpp"
#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"

TheGame* TheGame::instance = nullptr;

Sprite* testBackground = nullptr;
Sprite* titleText = nullptr;
Sprite* gameOverText = nullptr;
float m_timeSinceLastSpawn = 0.0f;
const float TIME_PER_SPAWN = 1.0f;

//-----------------------------------------------------------------------------------
void OnUpdateReceiveHelper(const NetSender& from, NetMessage& message)
{
#pragma todo("Remove this helper and use events for the messages")
    TheGame::instance->OnUpdateReceive(from, message);
}

//-----------------------------------------------------------------------------------
TheGame::TheGame()
    : m_debuggingControllerIndex(0)
{
    ResourceDatabase::instance = new ResourceDatabase();
    RegisterSprites();
    SetGameState(GameState::MAIN_MENU);
    InitializeMainMenuState();

    //Initialize networking subsystems
    RemoteCommandService::instance = new RemoteCommandService();
    Console::instance->RunCommand("nsinit");
    NetSession::instance->RegisterMessage((uint8_t)GAME_UPDATE, "Game Update", &OnUpdateReceiveHelper, (uint32_t)NetMessage::Option::NONE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->m_OnConnectionJoin.RegisterMethod(this, &TheGame::OnConnectionJoined);
    NetSession::instance->m_OnConnectionLeave.RegisterMethod(this, &TheGame::OnConnectionLeave);
    NetSession::instance->m_OnNetTick.RegisterMethod(this, &TheGame::OnNetTick);
    Console::instance->RunCommand("nsstart");
}

//-----------------------------------------------------------------------------------
TheGame::~TheGame()
{
    SetGameState(GameState::SHUTDOWN);
    delete ResourceDatabase::instance;
    ResourceDatabase::instance = nullptr;

    //Cleanup networking subsystems
    delete RemoteCommandService::instance;
    RemoteCommandService::instance = nullptr;
}

//-----------------------------------------------------------------------------------
void TheGame::OnConnectionJoined(NetConnection* cp)
{
    if (GetGameState() != GameState::PLAYING)
    {
        return;
    }
    if (cp->m_index == NetSession::instance->GetMyConnectionIndex())
    {
        m_localPlayer->m_netOwnerIndex = cp->m_index;
        return;
    }
    else
    {
        Player* player = new Player();
        player->m_netOwnerIndex = cp->m_index;
        m_players.push_back(player);
        m_entities.push_back(player);
    }
}

//-----------------------------------------------------------------------------------
void TheGame::OnConnectionLeave(NetConnection* cp)
{
    if (GetGameState() != GameState::PLAYING)
    {
        return;
    }
    else if (cp && cp->m_index != NetSession::instance->GetMyConnectionIndex())
    {
        uint8_t idx = cp->m_index;
        for (auto iter = m_players.begin(); iter != m_players.end(); ++iter)
        {
            Player* networkedPlayer = *iter;
            if (networkedPlayer->m_netOwnerIndex == idx)
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
}

//-----------------------------------------------------------------------------------
void TheGame::OnNetTick(NetConnection* cp)
{
    if (GetGameState() != GameState::PLAYING || cp->IsMyConnection() || !m_localPlayer)
    {
        return;
    }
    NetMessage update(GAME_UPDATE);
    update.Write<Vector2>(m_localPlayer->m_sprite->m_position);
    update.Write<float>(m_localPlayer->m_sprite->m_rotationDegrees);
    cp->SendMessage(update);
}

//-----------------------------------------------------------------------------------
void TheGame::OnUpdateReceive(const NetSender& from, NetMessage& message)
{
    if (GetGameState() != GameState::PLAYING)
    {
        return;
    }
    if (from.connection)
    {
        uint8_t idx = from.connection->m_index;
        for (Player* networkedPlayer : m_players)
        {
            if (networkedPlayer->m_netOwnerIndex == idx)
            {
                message.Read<Vector2>(networkedPlayer->m_sprite->m_position);
                message.Read<float>(networkedPlayer->m_sprite->m_rotationDegrees);
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------------------
void TheGame::Update(float deltaSeconds)
{
    SpriteGameRenderer::instance->Update(deltaSeconds);
    RemoteCommandService::instance->Update();
    if (InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::TILDE))
    {
        Console::instance->ToggleConsole();
    }
    if (Console::instance->IsActive())
    {
        return;
    }

#pragma todo("Reenable menu navigation once we have a more solid game flow")
    if (InputSystem::instance->m_controllers[m_debuggingControllerIndex]->JustPressed(XboxButton::START) || InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::ENTER))
    {
        switch (GetGameState())
        {
        case MAIN_MENU:
            SetGameState(PLAYING);
            InitializePlayingState();
            break;
        case PLAYING:
            //SetGameState(GAME_OVER);
            //InitializeGameOverState();
            break;
        case GAME_OVER:
            //SetGameState(MAIN_MENU);
            //InitializeMainMenuState();
            break;
        default:
            break;
        }
    }
    else if (InputSystem::instance->m_controllers[m_debuggingControllerIndex]->JustPressed(XboxButton::BACK) || InputSystem::instance->WasKeyJustPressed(InputSystem::ExtraKeys::BACKSPACE))
    {
        switch (GetGameState())
        {
        case GAME_OVER:
            //SetGameState(PLAYING);
            //InitializePlayingState();
            break;
        case PLAYING:
            //SetGameState(MAIN_MENU);
            //InitializeMainMenuState();
            break;
        default:
            break;
        }
    }

    switch (GetGameState())
    {
    case MAIN_MENU:
        UpdateMainMenu(deltaSeconds);
        break;
    case STARTUP:
        break;
    case PLAYING:
        UpdatePlaying(deltaSeconds);
        break;
    case PAUSED:
        //TODO: This will clean up all the game objects because of our callbacks, be careful here.
        break;
    case GAME_OVER:
        UpdateGameOver(deltaSeconds);
        break;
    case SHUTDOWN:
        break;
    case NUM_STATES:
        break;
    default:
        break;

    }
}

//-----------------------------------------------------------------------------------
void TheGame::Render() const
{
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_viewStack);
    ENSURE_NO_MATRIX_STACK_SIDE_EFFECTS(Renderer::instance->m_projStack);
    if (Console::instance->IsActive())
    {
        Console::instance->Render();
    }
    switch (GetGameState())
    {
    case MAIN_MENU:
        RenderMainMenu();
        break;
    case STARTUP:
        break;
    case PLAYING:
        RenderPlaying();
        break;
    case PAUSED:
        break;
    case GAME_OVER:
        RenderGameOver();
        break;
    case SHUTDOWN:
        break;
    case NUM_STATES:
        break;
    default:
        break;

    }

}

//-----------------------------------------------------------------------------------
//MAIN MENU/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeMainMenuState()
{
    titleText = new Sprite("TitleText", PLAYER_LAYER);
    titleText->m_scale = Vector2(1.0f, 1.0f);
    //TODO: SpriteGameRenderer::instance->AddEffectToLayer()
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupMainMenuState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupMainMenuState(unsigned int)
{
    delete titleText;
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateMainMenu(float deltaSeconds)
{
    UNUSED(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMainMenu() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::CERULEAN);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//PLAYING/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializePlayingState()
{
    testBackground = new Sprite("Map", BACKGROUND_LAYER);
    testBackground->m_scale = Vector2(1.0f, 1.0f);
    m_localPlayer = new Player();
    m_entities.push_back(m_localPlayer);
    m_players.push_back(m_localPlayer);
//     ItemBox* box1 = new ItemBox(Vector2(2.0f));
//     m_entities.push_back(box1);
//     ItemBox* box2 = new ItemBox(Vector2(1.0f));
//     m_entities.push_back(box2);
//     Grunt* g1 = new Grunt(Vector2(-2.0f));
//     m_entities.push_back(g1);
//     Grunt* g2 = new Grunt(Vector2(-1.0f));
//     m_entities.push_back(g2);
    SpriteGameRenderer::instance->SetWorldBounds(testBackground->GetBounds());
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupPlayingState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupPlayingState(unsigned int)
{
    for (Entity* ent : m_entities)
    {
        delete ent;
    }
    m_entities.clear();
    m_players.clear();
    delete testBackground;
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdatePlaying(float deltaSeconds)
{
    for (Entity* ent : m_entities)
    {
        ent->Update(deltaSeconds);
        for (Entity* other : m_entities)
        {
            if ((ent != other) && (ent->IsCollidingWith(other)))
            {
                ent->ResolveCollision(other);
            }
        }
    }
    for (Entity* ent : m_newEntities)
    {
        m_entities.push_back(ent);
    }
    m_newEntities.clear();
    for (auto iter = m_entities.begin(); iter != m_entities.end(); ++iter)
    {
        Entity* gameObject = *iter;
        if (gameObject->m_isDead)
        {
            delete gameObject;
            iter = m_entities.erase(iter);
        }
        if (iter == m_entities.end())
        {
            break;
        }
    }
    if (!m_localPlayer || m_localPlayer->m_isDead)
    {
        SetGameState(GAME_OVER);
        InitializeGameOverState();
    }
    else
    {
        SpriteGameRenderer::instance->SetCameraPosition(m_localPlayer->m_sprite->m_position);
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderPlaying() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::FEEDFACE);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
//GAME OVER/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializeGameOverState()
{
    gameOverText = new Sprite("GameOverText", PLAYER_LAYER);
    gameOverText->m_scale = Vector2(10.0f, 10.0f);
    //TODO: SpriteGameRenderer::instance->AddEffectToLayer()
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupGameOverState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupGameOverState(unsigned int)
{
    delete gameOverText;
}

//-----------------------------------------------------------------------------------
void TheGame::UpdateGameOver(float deltaSeconds)
{
    UNUSED(deltaSeconds);
}

//-----------------------------------------------------------------------------------
void TheGame::RenderGameOver() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::DISEASED);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::SpawnBullet(Ship* creator)
{
    m_newEntities.push_back(new Bullet(creator));
}

//-----------------------------------------------------------------------------------
void TheGame::SpawnPickup(const Vector2& spawnPosition)
{
    m_newEntities.push_back(new Pickup(spawnPosition));
}

//-----------------------------------------------------------------------------------
void TheGame::RegisterSprites()
{
    ResourceDatabase::instance->RegisterSprite("Map", "Data\\Images\\SymmetryCityMap.png");
    ResourceDatabase::instance->RegisterSprite("Player", "Data\\Images\\TempCapeBusiness.png");
    //-----------------------------------------------------------------------------------
    ResourceDatabase::instance->RegisterSprite("TitleText", "Data\\Images\\Title.png");
    ResourceDatabase::instance->RegisterSprite("GameOverText", "Data\\Images\\GameOver.png");
}

//-----------------------------------------------------------------------------------
CONSOLE_COMMAND(cport)
{
    UNUSED(args)
    TheGame::instance->m_debuggingControllerIndex += 1;
    Console::instance->PrintLine("Incremented Index", RGBA::DISEASED);
}