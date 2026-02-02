// GatorInvaders.cpp

#include <fstream>
#include <filesystem>
#include <cctype>

#include "GatorInvaders.h"

#include "Entities/Entity.h"
#include "Entities/Player.h"
#include "Entities/Enemy.h"
#include "Entities/Bullet.h"
#include "Entities/Obstacle.h"

#include "Core/Window.h"
#include "Core/Time.h"

#include "Graphics/Renderer.h"
#include "Graphics/TextRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"

#include "Input/Input.h"

#include "Physics/Physics.h"
#include "Physics/Collider.h"

#include "UI/Menu.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ------------------------------------------------------------
// Small helpers (file-local)
// ------------------------------------------------------------
static float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

static glm::vec4 GetBarrierPartUV(int partIndex, int parts, Texture* tex)
{
    const int W = tex->GetWidth();
    const int H = tex->GetHeight();

    // Split by pixel boundaries using rounding so all pixels are used exactly once
    const int x0 = (int)std::round((double)partIndex       * (double)W / (double)parts);
    const int x1 = (int)std::round((double)(partIndex + 1) * (double)W / (double)parts);

    // Texel-centered UVs to avoid nearest-filter sampling errors
    const float u0 = ((float)x0 + 0.5f) / (float)W;
    const float u1 = ((float)x1 - 0.5f) / (float)W;

    const float v0 = (0.5f) / (float)H;
    const float v1 = ((float)H - 0.5f) / (float)H;

    return glm::vec4(u0, v0, u1, v1);
}

static constexpr float UFO_RENDER_OFFSET_X = -7.0f; // +right / -left


// ------------------------------------------------------------

GatorInvaders::GatorInvaders()
    : m_State(GameState::MainMenu)
    , m_AudioInitialized(false)
    , m_PlayerStartPos(0.0f, -280.0f)
    , m_PlayerSpeed(300.0f)
    , m_PlayerMoveRange(550.0f)
    , m_EnemyRows(5)
    , m_EnemyColumns(11)
    , m_EnemyMoveTimer(0.0f)
    , m_EnemyMoveInterval(0.6f)
    , m_BaseMoveInterval(1.0f)
    , m_EnemyMoveDistance(20.0f)
    , m_EnemyDirection(1)
    , m_EnemiesShouldMoveDown(false)
    , m_CurrentEnemyFrame(0)
    , m_EnemyShootTimer(0.0f)
    , m_EnemyShootInterval(2.0f)
    , m_PlayerShootCooldown(0.0f)
    , m_PlayerShootTimer(0.0f)
    , m_CeilingY(350.0f)
    , m_UFOActive(false)
    , m_UFOSpawnTimer(0.0f)
    , m_UFOSpawnInterval(15.0f)
    , m_UFODirection(1)
    , m_Score(0)
    , m_Lives(3)
    , m_Level(1)
    , m_PlayerHitTimer(0.0f)
{
}

GatorInvaders::~GatorInvaders() = default;

void GatorInvaders::OnInit()
{
    std::cout << "\n";
    std::cout << "╔════════════════════════════════╗\n";
    std::cout << "║      GATOR INVADERS            ║\n";
    std::cout << "║      UF Gators Edition         ║\n";
    std::cout << "╚════════════════════════════════╝\n";
    std::cout << "\n";

    std::cout << "Controls:\n";
    std::cout << "  LEFT/RIGHT Arrows - Move\n";
    std::cout << "  A/D - Move\n";
    std::cout << "  SPACE - Shoot (only 1 bullet!)\n";
    std::cout << "  P - Pause\n";
    std::cout << "  R - Restart (when game over / next level)\n";
    std::cout << "  ESC - Pause / Back to Menu\n";
    std::cout << "  F11 - Toggle Fullscreen\n";
    std::cout << "  F1 - Toggle Debug Colliders\n\n";

    InitAudio();
    LoadTextures();

    GetWindow()->SetFullscreen(true);
    GetWindow()->SetTitle("Gator Invaders");
    GetWindow()->SetIcon("assets/textures/icon/icon.png");

    GetCamera()->SetPosition(glm::vec2(0.0f, 0.0f));
    GetCamera()->SetZoom(1.0f);

    // Invisible ceiling wall (kills player bullet)
    m_CeilingWall = std::make_unique<Obstacle>(
        glm::vec2(0.0f, m_CeilingY),
        glm::vec2(1400.0f, 20.0f)
    );
    m_CeilingWall->SetName("CeilingWall");
    m_CeilingWall->SetColor(glm::vec4(0, 0, 0, 0)); // invisible

    SetupMainMenu();
    SetupPauseMenu();
    SetupControlsMenu();

    SetupLeaderboardMenu();
    LoadLeaderboard();

    ShowMainMenu();
    Physics::SetDebugDraw(false);

    std::cout << "Game initialized. Showing main menu...\n";
}

void GatorInvaders::OnShutdown()
{
    std::cout << "Gator Invaders shutting down\n";
    AudioManager::StopMusic();

    m_Enemies.clear();
    m_EnemyRowScores.clear();
    m_EnemyBullets.clear();
    m_PlayerBullet.reset();
    m_Player.reset();
    m_CeilingWall.reset();

    // Clear barrier segments
    m_BarrierPartLookup.clear();
    for (auto& barrier : m_BarriersSplit)
    {
        for (auto& part : barrier.parts)
        {
            part.obstacle.reset();
        }
    }

    m_UFO.reset();

    m_MainMenu.reset();
    m_PauseMenu.reset();
    m_ControlsMenu.reset();
}

void GatorInvaders::InitAudio()
{
    m_AudioInitialized = true;

    AudioManager::SetMasterVolume(1.0f);
    AudioManager::SetMusicVolume(0.5f);
    AudioManager::SetSFXVolume(0.7f);

    std::cout << "Audio system ready!\n\n";
}

void GatorInvaders::LoadGameSounds()
{
    if (!m_AudioInitialized) return;

    static bool soundsLoaded = false;
    if (soundsLoaded) return;

    std::cout << "Loading game sounds...\n";

    AudioManager::LoadSound("shoot", "assets/audio/sfx/shoot.mp3");
    AudioManager::LoadSound("enemy_killed", "assets/audio/sfx/explosion.mp3");
    AudioManager::LoadSound("player_hit", "assets/audio/sfx/player_explosion.mp3");
    AudioManager::LoadSound("ufo", "assets/audio/sfx/ufo.mp3");
    AudioManager::LoadSound("ufo_killed", "assets/audio/sfx/ufo_explosion.mp3");
    AudioManager::LoadSound("game_over", "assets/audio/sfx/gameover.mp3");
    AudioManager::LoadSound("click", "assets/audio/sfx/click.mp3");

    soundsLoaded = true;
    std::cout << "Sounds loaded!\n";
}

void GatorInvaders::PlayGameMusic()
{
    if (!m_AudioInitialized) return;
    AudioManager::PlayMusic("assets/audio/music/background.mp3", true);
}

void GatorInvaders::StopGameMusic()
{
    if (!m_AudioInitialized) return;
    AudioManager::StopMusic();
}

void GatorInvaders::LoadTextures()
{
    std::cout << "Loading textures...\n";

    // Player
    m_PlayerTexture = std::make_unique<Texture>("assets/textures/player.png");

    // Enemies (2-frame sprite sheets)
    m_Enemy1Texture = std::make_unique<Texture>("assets/textures/gator_alien.png");
    m_Enemy2Texture = std::make_unique<Texture>("assets/textures/armored_orange_ufo.png");
    m_Enemy3Texture = std::make_unique<Texture>("assets/textures/blue_gator_ufos.png");

    // UFO (2-frame)
    m_UFOTexture = std::make_unique<Texture>("assets/textures/ufo.png");

    // Background
    m_BackgroundTexture = std::make_shared<Texture>("assets/textures/background.png");

    // Barrier damage stages (each is a full barrier image with 10 "columns")
    m_BarrierStageTextures[0] = std::make_unique<Texture>("assets/textures/barriers/barrier0.png");
    m_BarrierStageTextures[1] = std::make_unique<Texture>("assets/textures/barriers/barrier1.png");
    m_BarrierStageTextures[2] = std::make_unique<Texture>("assets/textures/barriers/barrier2.png");
    m_BarrierStageTextures[3] = std::make_unique<Texture>("assets/textures/barriers/barrier3.png");

    std::cout << "Textures loaded!\n";
}

void GatorInvaders::SetupMainMenu()
{
    m_MainMenu = std::make_unique<Menu>(); // MUST be first

    const float startY   = 80.0f;
    const float spacing  = 90.0f;
    const glm::vec2 size = { 250.0f, 60.0f };

    int i = 0;

    auto* startBtn = m_MainMenu->AddButton(
        "START GAME",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    startBtn->SetColors(
        glm::vec4(0.2f, 0.6f, 0.2f, 1.0f),
        glm::vec4(0.3f, 0.8f, 0.3f, 1.0f),
        glm::vec4(0.1f, 0.4f, 0.1f, 1.0f)
    );
    startBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); StartGame(); });

    auto* controlsBtn = m_MainMenu->AddButton(
        "CONTROLS",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    controlsBtn->SetColors(
        glm::vec4(0.2f, 0.2f, 0.6f, 1.0f),
        glm::vec4(0.3f, 0.3f, 0.8f, 1.0f),
        glm::vec4(0.1f, 0.1f, 0.4f, 1.0f)
    );
    controlsBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        ShowControlsMenu(false);
    });

    auto* leaderboardBtn = m_MainMenu->AddButton(
        "LEADERBOARD",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    leaderboardBtn->SetColors(
        glm::vec4(0.5f, 0.2f, 0.6f, 1.0f),
        glm::vec4(0.7f, 0.3f, 0.8f, 1.0f),
        glm::vec4(0.3f, 0.1f, 0.4f, 1.0f)
    );
    leaderboardBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        ShowLeaderboardMenu(false);
    });


    auto* quitBtn = m_MainMenu->AddButton(
        "QUIT",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    quitBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    quitBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); QuitGame(); });

    m_MainMenu->Show();
}


void GatorInvaders::SetupPauseMenu()
{
    m_PauseMenu = std::make_unique<Menu>(); // MUST be first

    const float startY   = 120.0f; // a little higher since there are 4 buttons
    const float spacing  = 90.0f;
    const glm::vec2 size = { 250.0f, 60.0f };

    int i = 0;

    auto* resumeBtn = m_PauseMenu->AddButton(
        "RESUME",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    resumeBtn->SetColors(
        glm::vec4(0.2f, 0.6f, 0.2f, 1.0f),
        glm::vec4(0.3f, 0.8f, 0.3f, 1.0f),
        glm::vec4(0.1f, 0.4f, 0.1f, 1.0f)
    );
    resumeBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); TogglePause(); });

    auto* controlsBtn = m_PauseMenu->AddButton(
        "CONTROLS",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    controlsBtn->SetColors(
        glm::vec4(0.2f, 0.2f, 0.6f, 1.0f),
        glm::vec4(0.3f, 0.3f, 0.8f, 1.0f),
        glm::vec4(0.1f, 0.1f, 0.4f, 1.0f)
    );
    controlsBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        ShowControlsMenu(true);
    });

    auto* restartBtn = m_PauseMenu->AddButton(
        "RESTART",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    restartBtn->SetColors(
        glm::vec4(0.6f, 0.6f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.8f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.4f, 0.1f, 1.0f)
    );
    restartBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        RestartGame();
        m_State = GameState::Playing;
        m_PauseMenu->Hide();
    });

    auto* menuBtn = m_PauseMenu->AddButton(
        "MAIN MENU",
        glm::vec2(0.0f, startY - i++ * spacing),
        size
    );
    menuBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    menuBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); ShowMainMenu(); });

    m_PauseMenu->Hide();
}

void GatorInvaders::SetupLeaderboardMenu()
{
    m_LeaderboardMenu = std::make_unique<Menu>();

    auto* backBtn = m_LeaderboardMenu->AddButton(
        "BACK",
        glm::vec2(0.0f, -240.0f),
        glm::vec2(250.0f, 60.0f)
    );
    backBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    backBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        CloseLeaderboardMenu();
    });

    m_LeaderboardMenu->Hide();
}

void GatorInvaders::ShowLeaderboardMenu(bool returnToPause)
{
    m_LeaderboardReturnToPause = returnToPause;
    m_State = GameState::Leaderboard;

    if (m_MainMenu) m_MainMenu->Hide();
    if (m_PauseMenu) m_PauseMenu->Hide();
    if (m_ControlsMenu) m_ControlsMenu->Hide();
    if (m_LeaderboardMenu) m_LeaderboardMenu->Show();
}

void GatorInvaders::CloseLeaderboardMenu()
{
    if (m_LeaderboardMenu) m_LeaderboardMenu->Hide();

    if (m_LeaderboardReturnToPause)
        ShowPauseMenu();
    else
        ShowMainMenu();
}

void GatorInvaders::ShowMainMenu()
{
    m_State = GameState::MainMenu;

    if (m_MainMenu) m_MainMenu->Show();
    if (m_PauseMenu) m_PauseMenu->Hide();

    StopGameMusic();

    // cleanup gameplay objects
    m_Enemies.clear();
    m_EnemyRowScores.clear();
    m_EnemyBullets.clear();
    m_PlayerBullet.reset();
    m_Player.reset();

    // Clear barrier segments
    m_BarrierPartLookup.clear();
    for (auto& barrier : m_BarriersSplit)
    {
        for (auto& part : barrier.parts)
        {
            part.obstacle.reset();
        }
    }

    m_UFO.reset();
    m_UFOActive = false;
    m_UFOSpawnTimer = 0.0f;
}

void GatorInvaders::ShowPauseMenu()
{
    m_State = GameState::Paused;
    if (m_PauseMenu) m_PauseMenu->Show();
    if (m_MainMenu) m_MainMenu->Hide();
}

void GatorInvaders::StartGame()
{
    m_State = GameState::Playing;
    if (m_MainMenu) m_MainMenu->Hide();
    if (m_PauseMenu) m_PauseMenu->Hide();

    LoadGameSounds();
    PlayGameMusic();

    m_Score = 0;
    m_Lives = 3;
    m_Level = 1;

    m_EnemyMoveInterval = m_BaseMoveInterval;
    m_EnemyShootInterval = 2.0f;

    m_PlayerShootTimer = 0.0f;
    m_EnemyMoveTimer = 0.0f;
    m_EnemyShootTimer = 0.0f;

    m_CurrentEnemyFrame = 0;

    // Create player
    m_Player = std::make_unique<Player>();
    m_Player->SetName("Player");
    m_Player->SetPosition(m_PlayerStartPos);
    m_Player->SetSize(glm::vec2(65.0f, 65.0f));
    m_Player->SetColor(glm::vec4(1, 1, 1, 1));
    m_Player->SetMaxSpeed(m_PlayerSpeed);
    m_Player->SetAcceleration(5000.0f);
    m_Player->SetDeceleration(5000.0f);
    m_Player->SetTexture("assets/textures/player.png");

    auto playerCollider = std::make_unique<BoxCollider>(m_Player.get(), m_Player->GetSize());
    playerCollider->SetTrigger(true);
    m_Player->SetCollider(std::move(playerCollider));

    SpawnEnemyGrid();
    SpawnBarriers();
}

void GatorInvaders::SetupControlsMenu()
{
    m_ControlsMenu = std::make_unique<Menu>();

    auto* backBtn = m_ControlsMenu->AddButton(
        "BACK",
        glm::vec2(0.0f, -220.0f),
        glm::vec2(250.0f, 60.0f)
    );
    backBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    backBtn->SetOnClick([this]() {
        AudioManager::PlaySFX("click");
        CloseControlsMenu();
    });

    m_ControlsMenu->Hide();
}

void GatorInvaders::ShowControlsMenu(bool returnToPause)
{
    m_ControlsReturnToPause = returnToPause;
    m_State = GameState::Controls;

    if (m_MainMenu) m_MainMenu->Hide();
    if (m_PauseMenu) m_PauseMenu->Hide();
    if (m_ControlsMenu) m_ControlsMenu->Show();
}

void GatorInvaders::CloseControlsMenu()
{
    if (m_ControlsMenu) m_ControlsMenu->Hide();

    if (m_ControlsReturnToPause)
    {
        ShowPauseMenu(); // goes back to paused state + shows pause menu
    }
    else
    {
        // Return to main menu state + show main menu
        m_State = GameState::MainMenu;
        if (m_MainMenu) m_MainMenu->Show();
        if (m_PauseMenu) m_PauseMenu->Hide();
    }
}

void GatorInvaders::QuitGame()
{
    glfwSetWindowShouldClose(GetWindow()->GetNativeWindow(), true);
}

void GatorInvaders::TogglePause()
{
    if (m_State == GameState::Playing)
    {
        ShowPauseMenu();
    }
    else if (m_State == GameState::Paused)
    {
        m_State = GameState::Playing;
        if (m_PauseMenu) m_PauseMenu->Hide();
    }
}

void GatorInvaders::SpawnEnemyGrid()
{
    m_Enemies.clear();
    m_EnemyRowScores.clear();

    // formation spacing
    const float startY = 250.0f;
    const float spacingX = 52.0f;
    const float spacingY = 44.0f;
    const float startX = -((m_EnemyColumns - 1) * spacingX) / 2.0f;

    m_Enemies.reserve((size_t)m_EnemyRows * (size_t)m_EnemyColumns);
    m_EnemyRowScores.reserve((size_t)m_EnemyRows * (size_t)m_EnemyColumns);

    for (int row = 0; row < m_EnemyRows; row++)
    {
        int rowScore = 10;
        Texture* sheet = nullptr;
        glm::vec2 enemySize(45.0f, 45.0f);

        auto computeSizeFromSheet = [](Texture* t, float desiredHeight)
        {
            float fw = t->GetWidth() / 2.0f;
            float fh = (float)t->GetHeight();
            float ar = (fh > 0.0f) ? (fw / fh) : 1.0f;
            return glm::vec2(desiredHeight * ar, desiredHeight);
        };

        if (row == 0)
        {
            rowScore = 40;
            sheet = m_Enemy1Texture.get();
            enemySize = computeSizeFromSheet(sheet, 45.0f);
        }
        else if (row == 1 || row == 2)
        {
            rowScore = 20;
            sheet = m_Enemy2Texture.get();
            enemySize = computeSizeFromSheet(sheet, 45.0f);
        }
        else
        {
            rowScore = 10;
            sheet = m_Enemy3Texture.get();
            enemySize = computeSizeFromSheet(sheet, 45.0f);
        }

        for (int col = 0; col < m_EnemyColumns; col++)
        {
            const float x = startX + col * spacingX;
            const float y = startY - row * spacingY;

            auto enemy = std::make_unique<Enemy>(glm::vec2(x, y), 0.0f);
            enemy->SetName("Enemy");
            enemy->SetSize(enemySize);
            enemy->SetDirection(glm::vec2(0, 0));
            enemy->SetColor(glm::vec4(1, 1, 1, 1));

            enemy->SetUseSpriteSheet(true);
            enemy->SetTexture(sheet);
            enemy->SetAnimationFrame(0);

            const float ENEMY_HITBOX_SCALE = 0.75f; // same for all enemies
            glm::vec2 colliderSize = enemySize * ENEMY_HITBOX_SCALE;

            auto collider = std::make_unique<BoxCollider>(enemy.get(), colliderSize);
            collider->SetTrigger(true);
            enemy->SetCollider(std::move(collider));


            m_Enemies.push_back(std::move(enemy));
            m_EnemyRowScores.push_back(rowScore);
        }
    }

    m_EnemyDirection = 1;
    m_EnemiesShouldMoveDown = false;

    // speed up per level
    m_EnemyMoveInterval = m_BaseMoveInterval * (1.0f - (m_Level - 1) * 0.1f);
    if (m_EnemyMoveInterval < 0.2f) m_EnemyMoveInterval = 0.2f;
}

void GatorInvaders::SpawnBarriers()
{
    // Clear lookup
    m_BarrierPartLookup.clear();

    // Bigger barriers for 4x3 grid - INCREASED SIZE AND SPACING
    const float barrierWidth  = 140.0f;
    const float barrierHeight = 80.0f;

    const float playWidth = 1400.0f;
    const float spacing   = playWidth / (BARRIER_COUNT + 1);
    const float barrierY  = -160.0f;

    const float partW = barrierWidth / (float)BARRIER_COLS;   // width per column
    const float partH = barrierHeight / (float)BARRIER_ROWS;  // height per row

    for (int b = 0; b < BARRIER_COUNT; b++)
    {
        float cx = -playWidth / 2.0f + spacing * (b + 1);

        Barrier& barrier = m_BarriersSplit[b];
        barrier.center = glm::vec2(cx, barrierY);
        barrier.size   = glm::vec2(barrierWidth, barrierHeight);

        int partIndex = 0;

        // Create 4x3 grid:
        // Row 0: [0][1][2][3] - all 4 columns
        // Row 1: [0][1][2][3] - all 4 columns
        // Row 2: [0][ ][ ][3] - only outer 2 columns
        for (int row = 0; row < BARRIER_ROWS; row++)
        {
            for (int col = 0; col < BARRIER_COLS; col++)
            {
                // Bottom row (row 2): only create parts for columns 0 and 3 (outer edges)
                if (row == BARRIER_ROWS - 1 && (col == 1 || col == 2))
                    continue;

                if (partIndex >= BARRIER_PARTS)
                    break;

                BarrierPart& part = barrier.parts[partIndex];
                part.stage = 0;
                part.broken = false;
                part.gridRow = row;
                part.gridCol = col;

                // Calculate position (origin at barrier center)
                float leftEdge = cx - barrierWidth * 0.5f;
                float topEdge  = barrierY + barrierHeight * 0.5f;

                float px = leftEdge + partW * (col + 0.5f);
                float py = topEdge - partH * (row + 0.5f);

                part.obstacle = std::make_unique<Obstacle>(
                    glm::vec2(px, py),
                    glm::vec2(partW, partH)
                );

                part.obstacle->SetName("BarrierPart");
                part.obstacle->SetColor(glm::vec4(1.0f));

                // Trigger: we handle bullet kill manually
                auto collider = std::make_unique<BoxCollider>(part.obstacle.get(), glm::vec2(partW, partH));
                collider->SetTrigger(true);
                part.obstacle->SetCollider(std::move(collider));

                // Map collider owner -> indices
                m_BarrierPartLookup[(Entity*)part.obstacle.get()] = { b, partIndex };

                partIndex++;
            }
        }
    }

    std::cout << "Spawned " << BARRIER_COUNT << " barriers (" << barrierWidth << "x" << barrierHeight
              << "), each with " << BARRIER_PARTS << " parts in a " << BARRIER_COLS << "x" << BARRIER_ROWS
              << " grid (hollow bottom middle), " << BARRIER_STAGES << " damage stages\n";
}

void GatorInvaders::ShootBullet()
{
    if (!m_Player) return;

    if (m_PlayerBullet && m_PlayerBullet->IsAlive())
        return;

    AudioManager::PlaySFX("shoot");

    glm::vec2 bulletPos = m_Player->GetPosition();
    bulletPos.y += 20.0f;

    m_PlayerBullet = std::make_unique<Bullet>(bulletPos, glm::vec2(0.0f, 1.0f), 600.0f);
    m_PlayerBullet->SetName("PlayerBullet");
    m_PlayerBullet->SetSize(glm::vec2(4.0f, 15.0f));
    m_PlayerBullet->SetColor(glm::vec4(1, 1, 1, 1));

    auto collider = std::make_unique<BoxCollider>(m_PlayerBullet.get(), glm::vec2(4.0f, 15.0f));
    collider->SetTrigger(true);
    m_PlayerBullet->SetCollider(std::move(collider));
}

void GatorInvaders::EnemyShoot()
{
    int aliveCount = 0;
    for (auto& e : m_Enemies)
        if (e->IsAlive()) aliveCount++;

    if (aliveCount <= 0) return;

    int pick = rand() % aliveCount;
    Enemy* shooter = nullptr;

    for (auto& e : m_Enemies)
    {
        if (!e->IsAlive()) continue;
        if (pick == 0) { shooter = e.get(); break; }
        pick--;
    }

    if (!shooter) return;

    auto bullet = std::make_unique<Bullet>(
        shooter->GetPosition(),
        glm::vec2(0.0f, -1.0f),
        300.0f
    );
    bullet->SetName("EnemyBullet");
    bullet->SetSize(glm::vec2(4.0f, 12.0f));
    bullet->SetColor(glm::vec4(1, 0, 0, 1));

    auto collider = std::make_unique<BoxCollider>(bullet.get(), glm::vec2(4.0f, 12.0f));
    collider->SetTrigger(true);
    bullet->SetCollider(std::move(collider));

    m_EnemyBullets.push_back(std::move(bullet));
    AudioManager::PlaySFX("shoot");
}

void GatorInvaders::OnUpdate(float deltaTime)
{
    // Menus / frozen states
    if (m_State == GameState::MainMenu)
    {
        if (m_MainMenu) m_MainMenu->Update(*GetCamera());
        return;
    }
    if (m_State == GameState::Paused)
    {
        if (m_PauseMenu) m_PauseMenu->Update(*GetCamera());
        return;
    }

    if (m_State == GameState::Controls)
    {
        if (m_ControlsMenu) m_ControlsMenu->Update(*GetCamera());
        return;
    }
    if (m_State == GameState::Leaderboard)
    {
        if (m_LeaderboardMenu) m_LeaderboardMenu->Update(*GetCamera());
        return;
    }

    if (m_State == GameState::EnterInitials)
    {
        // frozen while typing initials
        return;
    }


    if (m_State == GameState::GameOver || m_State == GameState::LevelComplete)
        return;

    if (m_State == GameState::PlayerHit)
    {
        m_PlayerHitTimer += deltaTime;
        if (m_PlayerHitTimer >= PLAYER_HIT_FREEZE_DURATION)
        {
            if (m_Player)
                m_Player->SetPosition(glm::vec2(-m_PlayerMoveRange + 50.0f, m_PlayerStartPos.y));

            m_State = GameState::Playing;
            m_PlayerHitTimer = 0.0f;
        }
        return;
    }

    // Level complete?
    if (m_Enemies.empty() ||
        std::all_of(m_Enemies.begin(), m_Enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return !e->IsAlive(); }))
    {
        m_State = GameState::LevelComplete;
        return;
    }

    // Player update + clamp
    if (m_Player)
    {
        m_Player->Update(deltaTime);

        glm::vec2 p = m_Player->GetPosition();
        p.x = glm::clamp(p.x, -m_PlayerMoveRange, m_PlayerMoveRange);
        p.y = m_PlayerStartPos.y;
        m_Player->SetPosition(p);
    }

    // Player bullet update
    if (m_PlayerBullet)
    {
        m_PlayerBullet->Update(deltaTime);
        if (!m_PlayerBullet->IsAlive())
            m_PlayerBullet.reset();
    }

    // Enemy bullets update
    for (auto& b : m_EnemyBullets)
        b->Update(deltaTime);

    // UFO update/spawn
    if (m_UFOActive && m_UFO)
    {
        m_UFO->Update(deltaTime);

        glm::vec2 u = m_UFO->GetPosition();
        u.x += (float)m_UFODirection * 250.0f * deltaTime;
        m_UFO->SetPosition(u);

        if (u.x > 700.0f || u.x < -700.0f)
        {
            m_UFOActive = false;
            AudioManager::StopSFX("ufo");
            m_UFO.reset();
        }
    }
    else
    {
        m_UFOSpawnTimer += deltaTime;
        if (m_UFOSpawnTimer >= m_UFOSpawnInterval)
        {
            m_UFOSpawnTimer = 0.0f;

            m_UFODirection = (rand() % 2 == 0) ? 1 : -1;
            float startX = (m_UFODirection > 0) ? -650.0f : 650.0f;

            m_UFO = std::make_unique<Enemy>(glm::vec2(startX, 290.0f), 0.0f);
            m_UFO->SetName("UFO");

            float fw = (float)m_UFOTexture->GetWidth();      // NO /2 (single frame)
            float fh = (float)m_UFOTexture->GetHeight();
            float ar = (fh > 0.0f) ? (fw / fh) : 1.0f;

            float height = 128.0f;
            glm::vec2 size(height * ar, height);

            m_UFO->SetSize(size);
            m_UFO->SetColor(glm::vec4(1, 1, 1, 1));
            m_UFO->SetUseSpriteSheet(false);                 // IMPORTANT
            m_UFO->SetTexture(m_UFOTexture.get());           // single texture
            // no animation frame needed



            glm::vec2 ufoColliderSize = size * 0.55f;
            auto col = std::make_unique<BoxCollider>(m_UFO.get(), ufoColliderSize);
            col->SetTrigger(true);
            m_UFO->SetCollider(std::move(col));


            m_UFOActive = true;

            AudioManager::SetSoundLooping("ufo", true);
            AudioManager::PlaySFX("ufo");
        }
    }

    // Enemy formation move + 2-frame animation
    m_EnemyMoveTimer += deltaTime;
    if (m_EnemyMoveTimer >= m_EnemyMoveInterval)
    {
        m_EnemyMoveTimer = 0.0f;
        m_CurrentEnemyFrame = 1 - m_CurrentEnemyFrame;

        bool hitEdge = false;

        for (auto& e : m_Enemies)
        {
            if (!e->IsAlive()) continue;
            float x = e->GetPosition().x;
            if ((m_EnemyDirection > 0 && x > 500.0f) ||
                (m_EnemyDirection < 0 && x < -500.0f))
            {
                hitEdge = true;
                break;
            }
        }

        for (auto& e : m_Enemies)
        {
            if (!e->IsAlive()) continue;

            e->SetAnimationFrame(m_CurrentEnemyFrame);

            glm::vec2 pos = e->GetPosition();
            if (hitEdge)
            {
                pos.y -= 20.0f;
                if (pos.y < -200.0f)
                {
                    GameOver();
                    return;
                }
            }
            else
            {
                pos.x += (float)m_EnemyDirection * m_EnemyMoveDistance;
            }
            e->SetPosition(pos);
        }


        if (hitEdge)
        {
            m_EnemyDirection *= -1;
            m_EnemyMoveInterval *= 0.95f;
            if (m_EnemyMoveInterval < 0.1f) m_EnemyMoveInterval = 0.1f;
        }
    }

    // Enemy shooting cadence
    m_EnemyShootTimer += deltaTime;
    if (m_EnemyShootTimer >= m_EnemyShootInterval)
    {
        m_EnemyShootTimer = 0.0f;
        EnemyShoot();
        m_EnemyShootInterval = 1.0f + (rand() % 100) / 100.0f;
    }

    CheckCollisions();
    CleanupDeadEntities();
}

void GatorInvaders::CleanupDeadEntities()
{
    m_EnemyBullets.erase(
        std::remove_if(m_EnemyBullets.begin(), m_EnemyBullets.end(),
            [](const std::unique_ptr<Bullet>& b) { return !b->IsAlive(); }),
        m_EnemyBullets.end()
    );
}

void GatorInvaders::CheckCollisions()
{
    // Helper to handle barrier part hits
    auto hitBarrierPart = [&](Entity* ent) -> bool
    {
        auto it = m_BarrierPartLookup.find(ent);
        if (it == m_BarrierPartLookup.end()) return false;

        int b = it->second.first;
        int p = it->second.second;

        BarrierPart& part = m_BarriersSplit[b].parts[p];
        if (part.broken) return false; // already broken => bullet should pass

        part.stage++;

        // stage 0->1->2->3 then break
        if (part.stage >= BARRIER_STAGES)
        {
            part.broken = true;
            // On the hit that breaks it, we still consume
            return true;
        }

        return true; // consumed (bullet dies) if it hit an unbroken piece
    };

    auto SegmentIntersectsAABB = [](glm::vec2 p0, glm::vec2 p1, glm::vec2 minB, glm::vec2 maxB) -> bool
    {
        // Liang–Barsky style parametric clip
        glm::vec2 d = p1 - p0;
        float tmin = 0.0f, tmax = 1.0f;

        auto clip = [&](float p, float q) {
            if (p == 0.0f) return q >= 0.0f;
            float r = q / p;
            if (p < 0.0f) { if (r > tmax) return false; if (r > tmin) tmin = r; }
            else         { if (r < tmin) return false; if (r < tmax) tmax = r; }
            return true;
        };

        if (!clip(-d.x, p0.x - minB.x)) return false;
        if (!clip( d.x, maxB.x - p0.x)) return false;
        if (!clip(-d.y, p0.y - minB.y)) return false;
        if (!clip( d.y, maxB.y - p0.y)) return false;
        return true;
    };

    if (m_PlayerBullet && m_PlayerBullet->IsAlive())
    {
        glm::vec2 p0 = m_PlayerBullet->GetPrevPosition();
        glm::vec2 p1 = m_PlayerBullet->GetPosition();

        for (size_t i = 0; i < m_Enemies.size(); i++)
        {
            auto& e = m_Enemies[i];
            if (!e || !e->IsAlive()) continue;

            auto* col = e->GetCollider();
            auto* box = dynamic_cast<BoxCollider*>(col);
            if (!box) continue;

            // BoxCollider stores its size (you call SetSize on it in Bullet, so it exists here too)
            glm::vec2 half = box->GetSize() * 0.5f;   // <-- if BoxCollider has GetSize()

            glm::vec2 c = e->GetPosition();
            glm::vec2 minB = c - half;
            glm::vec2 maxB = c + half;


            if (SegmentIntersectsAABB(p0, p1, minB, maxB))
            {
                e->Kill();
                if (auto* c = e->GetCollider())
                    c->SetEnabled(false);

                m_PlayerBullet->Kill();
                AudioManager::PlaySFX("enemy_killed");
                int points = (i < m_EnemyRowScores.size()) ? m_EnemyRowScores[i] : 10;
                m_Score += points;
                return; // bullet consumed
            }
        }
    }


    // ----------------------------
    // Player bullet collisions
    // ----------------------------
    if (m_PlayerBullet && m_PlayerBullet->IsAlive() && m_PlayerBullet->GetCollider())
    {
        auto hits = Physics::GetCollisions(m_PlayerBullet->GetCollider());

        for (auto* hitCol : hits)
        {
            Entity* ent = hitCol->GetOwner();
            if (!ent) continue;

            // Ceiling: despawn bullet
            if (ent->GetName() == "CeilingWall")
            {
                m_PlayerBullet->Kill();
                break;
            }

            // Barrier part: advance stage on hit
            if (ent->GetName() == "BarrierPart")
            {
                if (hitBarrierPart(ent))
                    m_PlayerBullet->Kill();

                break;
            }

            // Enemy: kill enemy, add score, despawn bullet
            if (ent->GetName() == "Enemy")
            {
                for (size_t i = 0; i < m_Enemies.size(); i++)
                {
                    if ((Entity*)m_Enemies[i].get() != ent) continue;
                    if (!m_Enemies[i]->IsAlive()) break;

                    m_Enemies[i]->Kill();
                    if (auto* c = m_Enemies[i]->GetCollider())
                        c->SetEnabled(false);

                    m_PlayerBullet->Kill();

                    AudioManager::PlaySFX("enemy_killed");

                    int points = (i < m_EnemyRowScores.size()) ? m_EnemyRowScores[i] : 10;
                    m_Score += points;
                    break;
                }
                break;
            }

            // UFO: bonus
            if (ent->GetName() == "UFO")
            {
                if (m_UFOActive && m_UFO && (Entity*)m_UFO.get() == ent && m_UFO->IsAlive())
                {
                    m_UFO->Kill();
                    m_PlayerBullet->Kill();
                    m_UFOActive = false;

                    AudioManager::StopSFX("ufo");
                    AudioManager::PlaySFX("ufo_killed");

                    static const int ufoScores[] = { 50, 100, 150, 300 };
                    int bonus = ufoScores[rand() % 4];
                    m_Score += bonus;

                    m_UFO.reset();
                }
                break;
            }
        }
    }

    // ----------------------------
    // Enemy bullets vs barriers & player
    // ----------------------------
    for (auto& bullet : m_EnemyBullets)
    {
        if (!bullet->IsAlive() || !bullet->GetCollider()) continue;

        auto hits = Physics::GetCollisions(bullet->GetCollider());

        for (auto* hitCol : hits)
        {
            Entity* ent = hitCol->GetOwner();
            if (!ent) continue;

            // Barrier part
            if (ent->GetName() == "BarrierPart")
            {
                if (hitBarrierPart(ent))
                    bullet->Kill();

                break;
            }

            // Enemy bullet hits player
            if (m_Player && ent == m_Player.get())
            {
                bullet->Kill();
                m_Lives--;
                AudioManager::PlaySFX("player_hit");

                if (m_Lives <= 0)
                {
                    GameOver();
                }
                else
                {
                    m_State = GameState::PlayerHit;
                    m_PlayerHitTimer = 0.0f;
                }
                break;
            }
        }
    }
}

void GatorInvaders::OnRender()
{
    // ------------------------------------------------------------
    // Background (always)
    // ------------------------------------------------------------
    if (m_BackgroundTexture)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();
        Renderer::DrawQuad(camPos, glm::vec2(1400.0f, 800.0f), m_BackgroundTexture.get());
    }

    // ------------------------------------------------------------
    // Main Menu
    // ------------------------------------------------------------
    if (m_State == GameState::MainMenu)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();

        glm::vec2 titlePos = glm::vec2(camPos.x - 280.0f, camPos.y + 220.0f);
        float titleScale = 3.5f;

        // --- White outline (4 directions) ---
        glm::vec4 outlineColor(1, 1, 1, 1);
        float o = 2.0f;

        TextRenderer::RenderText("GATOR INVADERS", titlePos + glm::vec2(-o,  0), titleScale, outlineColor);
        TextRenderer::RenderText("GATOR INVADERS", titlePos + glm::vec2( o,  0), titleScale, outlineColor);
        TextRenderer::RenderText("GATOR INVADERS", titlePos + glm::vec2( 0, -o), titleScale, outlineColor);
        TextRenderer::RenderText("GATOR INVADERS", titlePos + glm::vec2( 0,  o), titleScale, outlineColor);

        // --- Main text ---
        TextRenderer::RenderText(
            "GATOR INVADERS",
            titlePos,
            titleScale,
            glm::vec4(1.0f, 0.7f, 0.0f, 1.0f) // bright UF orange
        );

        glm::vec2 subPos = glm::vec2(camPos.x - 190.0f, camPos.y + 160.0f);
        float subScale = 1.5f;

        // Outline
        TextRenderer::RenderText("University of Florida", subPos + glm::vec2(-o,  0), subScale, outlineColor);
        TextRenderer::RenderText("University of Florida", subPos + glm::vec2( o,  0), subScale, outlineColor);
        TextRenderer::RenderText("University of Florida", subPos + glm::vec2( 0, -o), subScale, outlineColor);
        TextRenderer::RenderText("University of Florida", subPos + glm::vec2( 0,  o), subScale, outlineColor);

        // Main text
        TextRenderer::RenderText(
            "University of Florida",
            subPos,
            subScale,
            glm::vec4(0.1f, 0.5f, 1.0f, 1.0f) // bright blue
        );


        if (m_MainMenu) m_MainMenu->Render(*GetCamera());
        return;
    }

    // ------------------------------------------------------------
    // Controls screen
    // ------------------------------------------------------------
    if (m_State == GameState::Controls)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();

        TextRenderer::RenderText("CONTROLS",
            glm::vec2(camPos.x - 140.0f, camPos.y + 260.0f),
            3.0f,
            glm::vec4(1, 1, 1, 1));

        float x  = camPos.x - 420.0f;
        float y  = camPos.y + 170.0f;
        float s  = 1.6f;
        float dy = 38.0f;

        TextRenderer::RenderText("       Move Left:           LEFT Arrow / A",  glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Move Right:          RIGHT Arrow / D", glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Shoot:               SPACE",          glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Pause:               ESC / P",        glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Next Level:          R",              glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Fullscreen:          F11",            glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;
        TextRenderer::RenderText("       Show Hitboxes:       F1",             glm::vec2(x, y), s, glm::vec4(1,1,1,1)); y -= dy;

        TextRenderer::RenderText("Press ESC to go back",
            glm::vec2(camPos.x - 170.0f, camPos.y - 150.0f),
            1.5f,
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

        if (m_ControlsMenu) m_ControlsMenu->Render(*GetCamera());
        return;
    }

    if (m_State == GameState::Leaderboard)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();

        TextRenderer::RenderText("LEADERBOARD",
            glm::vec2(camPos.x - 220.0f, camPos.y + 280.0f),
            3.0f,
            glm::vec4(1, 1, 1, 1));

        float x = camPos.x - 260.0f;
        float y = camPos.y + 200.0f;
        float s = 1.8f;
        float dy = 35.0f;

        if (m_Leaderboard.empty())
        {
            TextRenderer::RenderText("No scores yet!",
                glm::vec2(camPos.x - 120.0f, camPos.y + 120.0f),
                1.8f,
                glm::vec4(0.9f, 0.9f, 0.9f, 1.0f));
        }
        else
        {
            for (int i = 0; i < (int)m_Leaderboard.size(); i++)
            {
                const auto& e = m_Leaderboard[i];
                std::string line = std::to_string(i + 1) + ".  " + e.initials + "    " + std::to_string(e.score);
                TextRenderer::RenderText(line, glm::vec2(x, y), s, glm::vec4(1,1,1,1));
                y -= dy;
            }
        }

        TextRenderer::RenderText("Press ESC to go back",
            glm::vec2(camPos.x - 170.0f, camPos.y - 170.0f),
            1.5f,
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

        if (m_LeaderboardMenu) m_LeaderboardMenu->Render(*GetCamera());
        return;
    }

    if (m_State == GameState::EnterInitials)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();

        TextRenderer::RenderText("NEW HIGH SCORE!",
            glm::vec2(camPos.x - 260.0f, camPos.y + 180.0f),
            2.8f,
            glm::vec4(1, 1, 0, 1));

        TextRenderer::RenderText("Enter your initials (3 letters):",
            glm::vec2(camPos.x - 320.0f, camPos.y + 90.0f),
            1.8f,
            glm::vec4(1, 1, 1, 1));

        std::string shown = m_InitialsInput;
        while (shown.size() < 3) shown += "_";

        TextRenderer::RenderText(shown,
            glm::vec2(camPos.x - 50.0f, camPos.y + 20.0f),
            3.0f,
            glm::vec4(1, 1, 1, 1));

        TextRenderer::RenderText("Press ENTER to submit",
            glm::vec2(camPos.x - 190.0f, camPos.y - 80.0f),
            1.6f,
            glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));

        return;
    }


    // ------------------------------------------------------------
    // Helper: draw barriers (used in Playing + Paused)
    // ------------------------------------------------------------
    auto DrawBarriers = [&]()
    {
        for (int b = 0; b < BARRIER_COUNT; b++)
        {
            Barrier& barrier = m_BarriersSplit[b];

            for (int p = 0; p < BARRIER_PARTS; p++)
            {
                BarrierPart& part = barrier.parts[p];
                if (!part.obstacle) continue;
                if (part.broken)    continue;

                int stage = std::max(0, std::min(BARRIER_STAGES - 1, part.stage));
                Texture* tex = m_BarrierStageTextures[stage].get();
                if (!tex) continue;

                // Each part shows its slice of the texture based on its grid row/col
                float u0 = (float)part.gridCol / (float)BARRIER_COLS;
                float u1 = (float)(part.gridCol + 1) / (float)BARRIER_COLS;

                // Flip V so row 0 is the TOP visually (matches how you place parts from topEdge downward)
                float v0 = 1.0f - (float)(part.gridRow + 1) / (float)BARRIER_ROWS;
                float v1 = 1.0f - (float)part.gridRow / (float)BARRIER_ROWS;

                glm::vec4 uv(u0, v0, u1, v1);

                Renderer::DrawQuadWithTexCoords(
                    part.obstacle->GetPosition(),
                    part.obstacle->GetSize(),
                    tex,
                    uv,
                    glm::vec4(1.0f)
                );
            }
        }
    };

    // ------------------------------------------------------------
    // WORLD RENDER (for Playing, Paused, GameOver, LevelComplete, PlayerHit)
    // ------------------------------------------------------------
    if (m_CeilingWall) m_CeilingWall->Render();

    // UFO (render offset so sprite looks centered, but restore position for physics)
    if (m_UFOActive && m_UFO && m_UFO->IsAlive())
    {
        glm::vec2 originalPos = m_UFO->GetPosition();
        glm::vec2 renderPos = originalPos;
        renderPos.x += UFO_RENDER_OFFSET_X;

        m_UFO->SetPosition(renderPos);
        m_UFO->Render();
        m_UFO->SetPosition(originalPos);
    }

    // Barriers
    DrawBarriers();

    // Enemies
    for (auto& e : m_Enemies)
        if (e && e->IsAlive())
            e->Render();

    // Bullets + player
    if (m_PlayerBullet) m_PlayerBullet->Render();
    for (auto& b : m_EnemyBullets) if (b) b->Render();
    if (m_Player) m_Player->Render();

    // ------------------------------------------------------------
    // UI (always during gameplay-ish states)
    // ------------------------------------------------------------
    glm::vec2 camPos = GetCamera()->GetPosition();

    TextRenderer::RenderText("Score:" + std::to_string(m_Score),
        glm::vec2(camPos.x - 600.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(1, 1, 1, 1));

    TextRenderer::RenderText("FPS:" + std::to_string(Time::GetFPS()),
        glm::vec2(camPos.x - 600.0f, camPos.y + 290.0f),
        1.5f,
        glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

    TextRenderer::RenderText("Lives:" + std::to_string(m_Lives),
        glm::vec2(camPos.x + 440.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(0, 1, 0, 1));

    TextRenderer::RenderText("Level:" + std::to_string(m_Level),
        glm::vec2(camPos.x - 70.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(1, 1, 0, 1));

    // ------------------------------------------------------------
    // Overlays / menus
    // ------------------------------------------------------------
    if (m_State == GameState::Paused)
    {
        glm::vec2 pausedPos = glm::vec2(camPos.x - 100.0f, camPos.y + 200.0f);
        float pausedScale = 3.0f;

        glm::vec4 outlineColor(1, 1, 1, 1);
        float o = 2.0f;

        // Outline
        TextRenderer::RenderText("PAUSED", pausedPos + glm::vec2(-o,  0), pausedScale, outlineColor);
        TextRenderer::RenderText("PAUSED", pausedPos + glm::vec2( o,  0), pausedScale, outlineColor);
        TextRenderer::RenderText("PAUSED", pausedPos + glm::vec2( 0, -o), pausedScale, outlineColor);
        TextRenderer::RenderText("PAUSED", pausedPos + glm::vec2( 0,  o), pausedScale, outlineColor);

        // Main text
        TextRenderer::RenderText(
            "PAUSED",
            pausedPos,
            pausedScale,
            glm::vec4(1.0f, 1.0f, 0.0f, 1.0f) // bright yellow
        );

        if (m_PauseMenu) m_PauseMenu->Render(*GetCamera());
        return;
    }

    if (m_State == GameState::GameOver)
    {
        TextRenderer::RenderText("GAME OVER",
            glm::vec2(camPos.x - 160.0f, camPos.y + 50.0f),
            3.0f,
            glm::vec4(1, 0, 0, 1));

        TextRenderer::RenderText("Press ESC for Menu",
            glm::vec2(camPos.x - 215.0f, camPos.y - 50.0f),
            2.0f,
            glm::vec4(1, 1, 1, 1));
    }
    else if (m_State == GameState::LevelComplete)
    {
        TextRenderer::RenderText("LEVEL COMPLETE!",
            glm::vec2(camPos.x - 235.0f, camPos.y + 50.0f),
            3.0f,
            glm::vec4(0, 1, 0, 1));

        TextRenderer::RenderText("Press R for Next Level",
            glm::vec2(camPos.x - 235.0f, camPos.y - 50.0f),
            2.0f,
            glm::vec4(1, 1, 1, 1));
    }
}


void GatorInvaders::OnInput(float deltaTime)
{
    // Global toggles
    if (Input::IsKeyJustPressed(KEY_F11))
        GetWindow()->ToggleFullscreen();

    if (Input::IsKeyJustPressed(KEY_F1))
    {
        bool cur = Physics::IsDebugDrawEnabled();
        Physics::SetDebugDraw(!cur);
    }

    // Menu input
    if (m_State == GameState::MainMenu)
    {
        if (Input::IsKeyJustPressed(KEY_ESCAPE))
            QuitGame();
        return;
    }

    // Pause menu input
    if (m_State == GameState::Paused)
    {
        if (Input::IsKeyJustPressed(KEY_ESCAPE) || Input::IsKeyJustPressed(KEY_P))
            TogglePause();
        return;
    }

    if (m_State == GameState::Controls)
    {
        if (Input::IsKeyJustPressed(KEY_ESCAPE))
            CloseControlsMenu();
        return;
    }

    if (m_State == GameState::Leaderboard)
    {
        if (Input::IsKeyJustPressed(KEY_ESCAPE))
            CloseLeaderboardMenu();
        return;
    }

    if (m_State == GameState::EnterInitials)
    {
        // Backspace
        if (Input::IsKeyJustPressed(KEY_BACKSPACE))
        {
            if (!m_InitialsInput.empty())
                m_InitialsInput.pop_back();
            return;
        }

        // Submit
        if (Input::IsKeyJustPressed(KEY_ENTER))
        {
            if (m_InitialsInput.size() == 3)
                SubmitInitialsEntry();
            return;
        }

        // Accept A-Z
        if (m_InitialsInput.size() < 3)
        {
            struct KeyLetter { int key; char c; };
            static const KeyLetter letters[] = {
                { KEY_A,'A'},{KEY_B,'B'},{KEY_C,'C'},{KEY_D,'D'},{KEY_E,'E'},{KEY_F,'F'},
                { KEY_G,'G'},{KEY_H,'H'},{KEY_I,'I'},{KEY_J,'J'},{KEY_K,'K'},{KEY_L,'L'},
                { KEY_M,'M'},{KEY_N,'N'},{KEY_O,'O'},{KEY_P,'P'},{KEY_Q,'Q'},{KEY_R,'R'},
                { KEY_S,'S'},{KEY_T,'T'},{KEY_U,'U'},{KEY_V,'V'},{KEY_W,'W'},{KEY_X,'X'},
                { KEY_Y,'Y'},{KEY_Z,'Z'}
            };

            for (const auto& kv : letters)
            {
                if (Input::IsKeyJustPressed(kv.key))
                {
                    m_InitialsInput.push_back(kv.c);
                    break;
                }
            }
        }

        return; // freeze everything else
    }

    // Game over input
    if (m_State == GameState::GameOver)
    {
        if (Input::IsKeyJustPressed(KEY_ESCAPE))
            ShowMainMenu();
        return;
    }

    // Level complete input
    if (m_State == GameState::LevelComplete)
    {
        if (Input::IsKeyJustPressed(KEY_R))
            NextLevel();

        if (Input::IsKeyJustPressed(KEY_ESCAPE))
            ShowMainMenu();

        return;
    }

    // Player hit freeze: no input
    if (m_State == GameState::PlayerHit)
        return;

    // Pause hotkeys in gameplay
    if (Input::IsKeyJustPressed(KEY_P) || Input::IsKeyJustPressed(KEY_ESCAPE))
    {
        TogglePause();
        return;
    }

    if (!m_Player) return;

    // Movement
    if (Input::IsKeyPressed(KEY_LEFT) || Input::IsKeyPressed(KEY_A))
    {
        glm::vec2 pos = m_Player->GetPosition();
        pos.x -= m_PlayerSpeed * deltaTime;
        m_Player->SetPosition(pos);
    }

    if (Input::IsKeyPressed(KEY_RIGHT) || Input::IsKeyPressed(KEY_D))
    {
        glm::vec2 pos = m_Player->GetPosition();
        pos.x += m_PlayerSpeed * deltaTime;
        m_Player->SetPosition(pos);
    }

    // Shooting (rate-limited)
    m_PlayerShootTimer += deltaTime;
    if (Input::IsKeyPressed(KEY_SPACE))
    {
        if (m_PlayerShootTimer >= PLAYER_SHOOT_RATE)
        {
            ShootBullet();
            m_PlayerShootTimer = 0.0f;
        }
    }
}

void GatorInvaders::NextLevel()
{
    m_Level++;
    m_State = GameState::Playing;

    m_PlayerBullet.reset();
    m_EnemyBullets.clear();

    if (m_Player)
        m_Player->SetPosition(m_PlayerStartPos);

    // ramp difficulty
    m_EnemyShootInterval *= 0.9f;
    if (m_EnemyShootInterval < 0.5f) m_EnemyShootInterval = 0.5f;

    m_CurrentEnemyFrame = 0;
    m_EnemyMoveTimer = 0.0f;
    m_EnemyShootTimer = 0.0f;

    SpawnEnemyGrid();
    SpawnBarriers();
}

void GatorInvaders::RestartGame()
{
    m_Score = 0;
    m_Lives = 3;
    m_Level = 1;

    m_State = GameState::Playing;
    m_CurrentEnemyFrame = 0;

    m_PlayerBullet.reset();
    m_EnemyBullets.clear();

    if (!m_Player)
    {
        m_Player = std::make_unique<Player>();
        m_Player->SetName("Player");
        m_Player->SetSize(glm::vec2(65.0f, 65.0f));
        m_Player->SetColor(glm::vec4(1, 1, 1, 1));
        m_Player->SetMaxSpeed(m_PlayerSpeed);
        m_Player->SetAcceleration(5000.0f);
        m_Player->SetDeceleration(5000.0f);
        m_Player->SetTexture("assets/textures/player.png");

        auto playerCollider = std::make_unique<BoxCollider>(m_Player.get(), m_Player->GetSize());
        playerCollider->SetTrigger(true);
        m_Player->SetCollider(std::move(playerCollider));
    }

    m_Player->SetPosition(m_PlayerStartPos);

    m_EnemyMoveInterval = m_BaseMoveInterval;
    m_EnemyShootInterval = 2.0f;

    m_EnemyMoveTimer = 0.0f;
    m_EnemyShootTimer = 0.0f;
    m_PlayerShootTimer = 0.0f;

    m_UFO.reset();
    m_UFOActive = false;
    m_UFOSpawnTimer = 0.0f;

    SpawnEnemyGrid();
    SpawnBarriers();
}

static std::string GetLeaderboardPath()
{
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA"); // Roaming
    std::filesystem::path base = appdata ? appdata : ".";
    std::filesystem::path dir  = base / "Gator Invaders";
#else
    // Fallback for non-Windows (fine for now)
    const char* home = std::getenv("HOME");
    std::filesystem::path base = home ? home : ".";
    std::filesystem::path dir  = base / ".gator_invaders";
#endif

    std::error_code ec;
    std::filesystem::create_directories(dir, ec); // ensure folder exists

    return (dir / "leaderboard.txt").string();
}


void GatorInvaders::LoadLeaderboard()
{
    m_Leaderboard.clear();

    std::ifstream in(GetLeaderboardPath());
    if (!in.is_open())
        return;

    // Format: ABC 12345
    std::string initials;
    int score = 0;

    while (in >> initials >> score)
    {
        if (initials.size() != 3) continue;
        m_Leaderboard.push_back({ initials, score });
    }

    std::sort(m_Leaderboard.begin(), m_Leaderboard.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score;
        });

    if ((int)m_Leaderboard.size() > 10)
        m_Leaderboard.resize(10);
}

void GatorInvaders::SaveLeaderboard()
{
    std::ofstream out(GetLeaderboardPath(), std::ios::trunc);
    if (!out.is_open())
        return;

    for (const auto& e : m_Leaderboard)
        out << e.initials << " " << e.score << "\n";
}


bool GatorInvaders::IsHighScore(int score) const
{
    if (score <= 0) return false;
    if ((int)m_Leaderboard.size() < 10) return true;
    return score > m_Leaderboard.back().score; // list is sorted desc
}

void GatorInvaders::BeginInitialsEntry()
{
    m_PendingScore = m_Score;
    m_InitialsInput.clear();
    m_State = GameState::EnterInitials;
}

void GatorInvaders::SubmitInitialsEntry()
{
    // Safety: normalize to exactly 3 uppercase letters
    for (char& c : m_InitialsInput)
        c = (char)std::toupper((unsigned char)c);

    if (m_InitialsInput.size() != 3)
        return;

    m_Leaderboard.push_back({ m_InitialsInput, m_PendingScore });

    // sort desc by score
    std::sort(m_Leaderboard.begin(), m_Leaderboard.end(),
        [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
            return a.score > b.score;
        });

    // keep top 10
    if ((int)m_Leaderboard.size() > 10)
        m_Leaderboard.resize(10);

    SaveLeaderboard();

    // After submit, show leaderboard
    ShowLeaderboardMenu(false);
}

void GatorInvaders::GameOver()
{
    AudioManager::PlaySFX("game_over");
    StopGameMusic();

    // If score qualifies, ask for initials; otherwise show normal game over
    if (IsHighScore(m_Score))
    {
        BeginInitialsEntry();
    }
    else
    {
        m_State = GameState::GameOver;
    }
}


// Header declares this, so define it
void GatorInvaders::DrawBackground(Camera* cam, const std::shared_ptr<Texture>& tex)
{
    if (!cam || !tex) return;
    Renderer::DrawQuad(cam->GetPosition(), glm::vec2(1400.0f, 800.0f), tex.get());
}