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
#include "Game/Entities/Link.hpp"
#include "Game/Entities/Bullet.hpp"
#include "Entities/ItemBox.hpp"
#include "Entities/Pickup.hpp"
#include "Engine/Net/RemoteCommandService.hpp"
#include "Engine/Net/UDPIP/NetConnection.hpp"
#include "Engine/Net/UDPIP/NetSession.hpp"
#include "Engine/Input/InputDevices.hpp"
#include "Engine/Net/NetSystem.hpp"
#include "Engine/Time/Time.hpp"
#include "Game/HostSimulation.hpp"
#include "Game/ClientSimulation.hpp"

TheGame* TheGame::instance = nullptr;

Sprite* testBackground = nullptr;
Sprite* titleText = nullptr;
Sprite* gameOverText = nullptr;
float m_timeSinceLastSpawn = 0.0f;
const float TIME_PER_SPAWN = 1.0f;

//-----------------------------------------------------------------------------------
void OnClientToHostUpdateReceiveHelper(const NetSender& from, NetMessage& message)
{
#pragma todo("Remove this helper and use events for the messages")
    if (TheGame::instance->m_host)
    {
        TheGame::instance->m_host->OnUpdateFromClientReceived(from, message);
    }
}

//-----------------------------------------------------------------------------------
void OnHostToClientUpdateReceiveHelper(const NetSender& from, NetMessage& message)
{
    if (TheGame::instance->m_client)
    {
        TheGame::instance->m_client->OnUpdateFromHostReceived(from, message);
    }
}

//-----------------------------------------------------------------------------------
void OnPlayerCreate(const NetSender& from, NetMessage& message)
{
    if (TheGame::instance->m_host)
    {
        TheGame::instance->m_host->OnPlayerCreate(from, message);
    }
    if (TheGame::instance->m_client)
    {
        TheGame::instance->m_client->OnPlayerCreate(from, message);
    }
}

//-----------------------------------------------------------------------------------
void OnPlayerDestroy(const NetSender& from, NetMessage& message)
{
    if (TheGame::instance->m_host)
    {
        TheGame::instance->m_host->OnPlayerDestroy(from, message);
    }
    if (TheGame::instance->m_client)
    {
        TheGame::instance->m_client->OnPlayerDestroy(from, message);
    }
}

//-----------------------------------------------------------------------------------
void OnPlayerAttack(const NetSender& from, NetMessage& message)
{
    if (TheGame::instance->m_host)
    {
        TheGame::instance->m_host->OnPlayerAttack(from, message);
    }
    if (TheGame::instance->m_client)
    {
        TheGame::instance->m_client->OnPlayerAttack(from, message);
    }
}

//-----------------------------------------------------------------------------------
void OnPlayerDamaged(const NetSender& from, NetMessage& message)
{
    if (TheGame::instance->m_host)
    {
        TheGame::instance->m_host->OnPlayerDamaged(from, message);
    }
    if (TheGame::instance->m_client)
    {
        TheGame::instance->m_client->OnPlayerDamaged(from, message);
    }
}

//-----------------------------------------------------------------------------------
TheGame::TheGame()
    : m_debuggingControllerIndex(0)
    , m_host(nullptr)
    , m_client(nullptr)
{
    //Get a random timestamp seed.
    LARGE_INTEGER currentCount;
    QueryPerformanceCounter(&currentCount);
    srand((unsigned int)currentCount.QuadPart);

    //Initialize resources and keybindings.
    ResourceDatabase::instance = new ResourceDatabase();
    RegisterSprites();
    RegisterParticleSystems();
    InitializeKeyMappings();
    SetGameState(GameState::MAIN_MENU);
    InitializeMainMenuState();

    //Initialize networking subsystems.
    RemoteCommandService::instance = new RemoteCommandService();
    Console::instance->RunCommand("nsinit");
    NetSession::instance->RegisterMessage((uint8_t)CLIENT_TO_HOST_UPDATE, "Client to Host Update", &OnClientToHostUpdateReceiveHelper, (uint32_t)NetMessage::Option::NONE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)HOST_TO_CLIENT_UPDATE, "Host to Client Update", &OnHostToClientUpdateReceiveHelper, (uint32_t)NetMessage::Option::NONE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)PLAYER_CREATE, "Player Create", &OnPlayerCreate, (uint32_t)NetMessage::Option::RELIABLE | (uint32_t)NetMessage::Option::INORDER, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)PLAYER_DESTROY, "Player Destroy", &OnPlayerDestroy, (uint32_t)NetMessage::Option::RELIABLE | (uint32_t)NetMessage::Option::INORDER, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)PLAYER_ATTACK, "Player Attack", &OnPlayerAttack, (uint32_t)NetMessage::Option::RELIABLE, (uint32_t)NetMessage::Control::NONE);
    NetSession::instance->RegisterMessage((uint8_t)PLAYER_DAMAGED, "Player Damaged", &OnPlayerDamaged, (uint32_t)NetMessage::Option::RELIABLE, (uint32_t)NetMessage::Control::NONE);
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

    if (m_host)
    {
        delete m_host;
    }
    if (m_client)
    {
        delete m_client;
    }

    //Cleanup networking subsystems
    delete RemoteCommandService::instance;
    RemoteCommandService::instance = nullptr;
}

//-----------------------------------------------------------------------------------
void TheGame::OnConnectionJoined(NetConnection* cp)
{
    if (m_host)
    {
        m_host->OnConnectionJoined(cp);
    }
    if (m_client)
    {
        m_client->OnConnectionJoined(cp);
    }
}

//-----------------------------------------------------------------------------------
void TheGame::OnConnectionLeave(NetConnection* cp)
{
    if (m_host)
    {
        m_host->OnConnectionLeave(cp);
    }
    if (m_client)
    {
        m_client->OnConnectionLeave(cp);
    }
}

//-----------------------------------------------------------------------------------
void TheGame::OnNetTick(NetConnection* cp)
{
    if (m_host)
    {
        m_host->SendNetHostUpdate(cp);
    }
    if (m_client)
    {
        m_client->SendNetClientUpdate(cp);
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

    switch (GetGameState())
    {
    case MAIN_MENU:
        UpdateMainMenu(deltaSeconds);
        break;
    case PLAYING:
        UpdatePlaying(deltaSeconds);
        break;
    case GAME_OVER:
        UpdateGameOver(deltaSeconds);
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

    if (m_gameplayMapping.IsDown("Host"))
    {
        m_host = new HostSimulation();
        m_client = new ClientSimulation();
        Console::instance->RunCommand("nethost Host");
        while (!NetSession::instance->AmIConnected())
        {
            Sleep(100);
        }

        //Force creation of the host's player and potentially local client player.
        NetMessage message(GameNetMessages::PLAYER_CREATE);
        message.Write<uint8_t>(NetSession::instance->m_hostConnection->m_index);
        message.Write<unsigned int>(RGBA::GetRandom().ToUnsignedInt());
        NetSession::instance->m_hostConnection->SendMessage(message);
        
        SetGameState(PLAYING);
        InitializePlayingState();
    }
    if (m_gameplayMapping.IsDown("Join"))
    {
        m_client = new ClientSimulation();
        Console::instance->RunCommand(Stringf("netjoin client %s", NetSystem::SockAddrToString(NetSystem::GetLocalHostAddressUDP("4334"))));
        
        SetGameState(PLAYING);
        InitializePlayingState();
    }
}

//-----------------------------------------------------------------------------------
void TheGame::RenderMainMenu() const
{
    SpriteGameRenderer::instance->SetClearColor(RGBA::CERULEAN);
    SpriteGameRenderer::instance->Render();
}

//-----------------------------------------------------------------------------------
void TheGame::InitializeKeyMappings()
{
    KeyboardInputDevice* keyboard = InputSystem::instance->m_keyboardDevice;
    m_gameplayMapping.AddInputAxis("Up", keyboard->FindValue('W'), keyboard->FindValue('S'));
    m_gameplayMapping.AddInputAxis("Right", keyboard->FindValue('D'), keyboard->FindValue('A'));
    m_gameplayMapping.AddInputValue("Attack", keyboard->FindValue(' '));
    m_gameplayMapping.AddInputValue("Host", keyboard->FindValue('H'));
    m_gameplayMapping.AddInputValue("Join", keyboard->FindValue('J'));
}

//-----------------------------------------------------------------------------------
//PLAYING/////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
void TheGame::InitializePlayingState()
{
    testBackground = new Sprite("Map", BACKGROUND_LAYER);
    testBackground->m_scale = Vector2(1.0f, 1.0f);
    SpriteGameRenderer::instance->SetWorldBounds(testBackground->GetBounds());
    OnStateSwitch.RegisterMethod(this, &TheGame::CleanupPlayingState);
}

//-----------------------------------------------------------------------------------
void TheGame::CleanupPlayingState(unsigned int)
{
    delete testBackground;
    SpriteGameRenderer::instance->SetCameraPosition(Vector2::ZERO);
}

//-----------------------------------------------------------------------------------
void TheGame::UpdatePlaying(float deltaSeconds)
{
    if (NetSession::instance->IsHost())
    {
        m_host->Update(deltaSeconds);
    }
    if (m_client)
    {
        m_client->Update(deltaSeconds);
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
void TheGame::RegisterSprites()
{
    ResourceDatabase::instance->RegisterSprite("Map", "Data\\Images\\SymmetryCityMap.png");
    ResourceDatabase::instance->RegisterSprite("pDown", "Data\\Images\\standingDown.png");
    ResourceDatabase::instance->RegisterSprite("pUp", "Data\\Images\\standingUp.png");
    ResourceDatabase::instance->RegisterSprite("pRight", "Data\\Images\\standingRight.png");
    ResourceDatabase::instance->RegisterSprite("pLeft", "Data\\Images\\standingLeft.png");
    ResourceDatabase::instance->RegisterSprite("swordSwing", "Data\\Images\\swordSwing.png");

    ResourceDatabase::instance->RegisterSprite("TitleText", "Data\\Images\\Title.png");
    ResourceDatabase::instance->RegisterSprite("GameOverText", "Data\\Images\\GameOver.png");
}

//-----------------------------------------------------------------------------------
void TheGame::RegisterParticleSystems()
{
    ParticleSystemDefinition* swordAttackSystem = ResourceDatabase::instance->RegisterParticleSystem("SwordAttack", ONE_SHOT);
    ParticleEmitterDefinition* swordAttackEmitter = new ParticleEmitterDefinition(ResourceDatabase::instance->GetSpriteResource("swordSwing"));
    swordAttackEmitter->m_initialNumParticlesSpawn = 1;
    swordAttackEmitter->m_lifetimePerParticle = 0.1f;
    swordAttackEmitter->m_material = swordAttackEmitter->m_spriteResource->m_defaultMaterial;
    swordAttackEmitter->m_particlesPerSecond = 0.0f;
    swordAttackEmitter->m_fadeoutEnabled = false;
    swordAttackSystem->AddEmitter(swordAttackEmitter);
}