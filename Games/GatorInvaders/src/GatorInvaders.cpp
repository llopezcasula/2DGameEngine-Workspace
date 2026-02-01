// GatorInvaders.cpp (full file - barrier fix using REAL UV slices for your barrier.png)

#include "SpaceInvadersGame.h"

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

// ------------------------------------------------------------
// Small helpers (file-local)
// ------------------------------------------------------------
static float Clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

// IMPORTANT:
// Your barrier.png is NOT an evenly-spaced 10-column sprite sheet.
// It contains 5 frames with padding, so we use hard-coded UV boxes for each frame.
// These UVs sample the correct parts so barriers won't look "cropped" or "flipped".
static glm::vec4 GetBarrierFrameUV(int frameIndex)
{
    // 5 frames: full -> damaged -> more damaged -> etc
    static const glm::vec4 UVS[5] =
    {
        // (u0, v0, u1, v1)
        {0.015625f, 0.355469f, 0.192057f, 0.577148f}, // frame 0 (full)
        {0.199219f, 0.355469f, 0.382812f, 0.581055f}, // frame 1
        {0.391276f, 0.355469f, 0.578776f, 0.581055f}, // frame 2
        {0.583984f, 0.355469f, 0.763672f, 0.583984f}, // frame 3
        {0.771484f, 0.356445f, 0.967448f, 0.576172f}, // frame 4 (most damaged)
    };

    frameIndex = std::clamp(frameIndex, 0, 4);

    // tiny inset to reduce bleeding from neighboring pixels
    const float insetU = 0.0008f;
    const float insetV = 0.0008f;

    glm::vec4 uv = UVS[frameIndex];
    uv.x += insetU; uv.z -= insetU;
    uv.y += insetV; uv.w -= insetV;
    return uv;
}

// ------------------------------------------------------------

SpaceInvadersGame::SpaceInvadersGame()
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

SpaceInvadersGame::~SpaceInvadersGame() = default;

void SpaceInvadersGame::OnInit()
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

    ShowMainMenu();
    Physics::SetDebugDraw(false);

    std::cout << "Game initialized. Showing main menu...\n";
}

void SpaceInvadersGame::OnShutdown()
{
    std::cout << "Gator Invaders shutting down\n";
    AudioManager::StopMusic();

    m_Enemies.clear();
    m_EnemyRowScores.clear();
    m_EnemyBullets.clear();
    m_PlayerBullet.reset();
    m_Player.reset();
    m_CeilingWall.reset();
    m_Barriers.clear();
    m_BarrierHealth.clear();
    m_UFO.reset();

    m_MainMenu.reset();
    m_PauseMenu.reset();
}

void SpaceInvadersGame::InitAudio()
{
    m_AudioInitialized = true;

    AudioManager::SetMasterVolume(1.0f);
    AudioManager::SetMusicVolume(0.5f);
    AudioManager::SetSFXVolume(0.7f);

    std::cout << "Audio system ready!\n\n";
}

void SpaceInvadersGame::LoadGameSounds()
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

void SpaceInvadersGame::PlayGameMusic()
{
    if (!m_AudioInitialized) return;
    AudioManager::PlayMusic("assets/audio/music/background.mp3", true);
}

void SpaceInvadersGame::StopGameMusic()
{
    if (!m_AudioInitialized) return;
    AudioManager::StopMusic();
}

void SpaceInvadersGame::LoadTextures()
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

    // Barriers (your file is 5 frames with padding)
    m_BarrierSheet = std::make_unique<Texture>("assets/textures/barrier.png");

    std::cout << "Textures loaded!\n";
}

void SpaceInvadersGame::SetupMainMenu()
{
    m_MainMenu = std::make_unique<Menu>();

    auto* startBtn = m_MainMenu->AddButton(
        "START GAME",
        glm::vec2(0.0f, 50.0f),
        glm::vec2(250.0f, 60.0f)
    );
    startBtn->SetColors(
        glm::vec4(0.2f, 0.6f, 0.2f, 1.0f),
        glm::vec4(0.3f, 0.8f, 0.3f, 1.0f),
        glm::vec4(0.1f, 0.4f, 0.1f, 1.0f)
    );
    startBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); StartGame(); });

    auto* quitBtn = m_MainMenu->AddButton(
        "QUIT",
        glm::vec2(0.0f, -50.0f),
        glm::vec2(250.0f, 60.0f)
    );
    quitBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    quitBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); QuitGame(); });

    m_MainMenu->Show();
}

void SpaceInvadersGame::SetupPauseMenu()
{
    m_PauseMenu = std::make_unique<Menu>();

    auto* resumeBtn = m_PauseMenu->AddButton(
        "RESUME",
        glm::vec2(0.0f, 80.0f),
        glm::vec2(250.0f, 60.0f)
    );
    resumeBtn->SetColors(
        glm::vec4(0.2f, 0.6f, 0.2f, 1.0f),
        glm::vec4(0.3f, 0.8f, 0.3f, 1.0f),
        glm::vec4(0.1f, 0.4f, 0.1f, 1.0f)
    );
    resumeBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); TogglePause(); });

    auto* restartBtn = m_PauseMenu->AddButton(
        "RESTART",
        glm::vec2(0.0f, 0.0f),
        glm::vec2(250.0f, 60.0f)
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
        glm::vec2(0.0f, -80.0f),
        glm::vec2(250.0f, 60.0f)
    );
    menuBtn->SetColors(
        glm::vec4(0.6f, 0.2f, 0.2f, 1.0f),
        glm::vec4(0.8f, 0.3f, 0.3f, 1.0f),
        glm::vec4(0.4f, 0.1f, 0.1f, 1.0f)
    );
    menuBtn->SetOnClick([this]() { AudioManager::PlaySFX("click"); ShowMainMenu(); });

    m_PauseMenu->Hide();
}

void SpaceInvadersGame::ShowMainMenu()
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
    m_Barriers.clear();
    m_BarrierHealth.clear();
    m_UFO.reset();
    m_UFOActive = false;
    m_UFOSpawnTimer = 0.0f;
}

void SpaceInvadersGame::ShowPauseMenu()
{
    m_State = GameState::Paused;
    if (m_PauseMenu) m_PauseMenu->Show();
    if (m_MainMenu) m_MainMenu->Hide();
}

void SpaceInvadersGame::StartGame()
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

void SpaceInvadersGame::QuitGame()
{
    glfwSetWindowShouldClose(GetWindow()->GetNativeWindow(), true);
}

void SpaceInvadersGame::TogglePause()
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

void SpaceInvadersGame::SpawnEnemyGrid()
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
            enemySize = computeSizeFromSheet(sheet, 55.0f);
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

            auto collider = std::make_unique<BoxCollider>(enemy.get(), enemySize);
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

void SpaceInvadersGame::SpawnBarriers()
{
    m_Barriers.clear();
    m_BarrierHealth.clear();

    const int numBarriers = 4;

    const float barrierWidth  = 180.0f;
    const float barrierHeight = 90.0f;

    const float playWidth = 1200.0f;
    const float spacing   = playWidth / (numBarriers + 1);
    const float barrierY  = -200.0f;

    m_Barriers.reserve(numBarriers);
    m_BarrierHealth.reserve(numBarriers);

    for (int i = 0; i < numBarriers; i++)
    {
        float x = -playWidth / 2.0f + spacing * (i + 1);

        auto barrier = std::make_unique<Obstacle>(
            glm::vec2(x, barrierY),
            glm::vec2(barrierWidth, barrierHeight)
        );
        barrier->SetName("Barrier");
        barrier->SetColor(glm::vec4(1.0f));

        // Trigger collider so bullets pass through but register hits
        auto col = std::make_unique<BoxCollider>(barrier.get(), glm::vec2(barrierWidth, barrierHeight));
        col->SetTrigger(true);
        barrier->SetCollider(std::move(col));

        m_Barriers.push_back(std::move(barrier));
        m_BarrierHealth.push_back(MAX_BARRIER_HEALTH);
    }

    std::cout << "Spawned barriers size: " << barrierWidth << " x " << barrierHeight << "\n";
}

void SpaceInvadersGame::UpdateBarrierColors()
{
    for (size_t i = 0; i < m_Barriers.size(); i++)
    {
        if (m_BarrierHealth[i] <= 0) continue;

        float hp01 = (float)m_BarrierHealth[i] / (float)MAX_BARRIER_HEALTH;
        hp01 = Clamp01(hp01);

        float r = 1.0f - hp01;
        float g = hp01;

        m_Barriers[i]->SetColor(glm::vec4(r, g, 0.0f, 1.0f));
    }
}

void SpaceInvadersGame::ShootBullet()
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

void SpaceInvadersGame::EnemyShoot()
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

void SpaceInvadersGame::OnUpdate(float deltaTime)
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

            float fw = m_UFOTexture->GetWidth() / 2.0f;
            float fh = (float)m_UFOTexture->GetHeight();
            float ar = (fh > 0.0f) ? (fw / fh) : 1.0f;

            float height = 128.0f;
            glm::vec2 size(height * ar, height);

            m_UFO->SetSize(size);
            m_UFO->SetColor(glm::vec4(1, 1, 1, 1));
            m_UFO->SetUseSpriteSheet(true);
            m_UFO->SetTexture(m_UFOTexture.get());
            m_UFO->SetAnimationFrame(0);

            auto col = std::make_unique<BoxCollider>(m_UFO.get(), size);
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

        if (m_UFOActive && m_UFO)
            m_UFO->SetAnimationFrame(m_CurrentEnemyFrame);

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

        // randomize a bit
        m_EnemyShootInterval = 1.0f + (rand() % 100) / 100.0f;
    }

    CheckCollisions();
    CleanupDeadEntities();
}

void SpaceInvadersGame::CheckCollisions()
{
    // Player bullet collisions
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

            // Barrier: damage barrier and despawn bullet
            if (ent->GetName() == "Barrier")
            {
                for (size_t i = 0; i < m_Barriers.size(); i++)
                {
                    if ((Entity*)m_Barriers[i].get() != ent) continue;

                    if (m_BarrierHealth[i] > 0)
                    {
                        m_BarrierHealth[i]--;
                        m_PlayerBullet->Kill();

                        if (m_BarrierHealth[i] <= 0)
                            m_Barriers[i]->SetActive(false);
                    }
                    break;
                }
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
                    m_PlayerBullet->Kill();

                    AudioManager::PlaySFX("enemy_killed");

                    int points = (i < m_EnemyRowScores.size()) ? m_EnemyRowScores[i] : 10;
                    m_Score += points;
                    break;
                }
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
                    break;
                }
            }
        }
    }

    // Enemy bullets vs barriers & player
    for (auto& bullet : m_EnemyBullets)
    {
        if (!bullet->IsAlive() || !bullet->GetCollider()) continue;

        auto hits = Physics::GetCollisions(bullet->GetCollider());

        for (auto* hitCol : hits)
        {
            Entity* ent = hitCol->GetOwner();
            if (!ent) continue;

            if (ent->GetName() == "Barrier")
            {
                for (size_t i = 0; i < m_Barriers.size(); i++)
                {
                    if ((Entity*)m_Barriers[i].get() != ent) continue;

                    if (m_BarrierHealth[i] > 0)
                    {
                        m_BarrierHealth[i]--;
                        bullet->Kill();

                        if (m_BarrierHealth[i] <= 0)
                            m_Barriers[i]->SetActive(false);
                    }
                    break;
                }
                break;
            }

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

void SpaceInvadersGame::CleanupDeadEntities()
{
    m_EnemyBullets.erase(
        std::remove_if(m_EnemyBullets.begin(), m_EnemyBullets.end(),
            [](const std::unique_ptr<Bullet>& b) { return !b->IsAlive(); }),
        m_EnemyBullets.end()
    );
}

void SpaceInvadersGame::OnRender()
{
    // Background
    if (m_BackgroundTexture)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();
        Renderer::DrawQuad(camPos, glm::vec2(1400.0f, 800.0f), m_BackgroundTexture.get());
    }

    // Main menu screen
    if (m_State == GameState::MainMenu)
    {
        glm::vec2 camPos = GetCamera()->GetPosition();

        TextRenderer::RenderText("GATOR INVADERS",
            glm::vec2(camPos.x - 280.0f, camPos.y + 200.0f),
            3.5f,
            glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

        TextRenderer::RenderText("University of Florida",
            glm::vec2(camPos.x - 190.0f, camPos.y + 140.0f),
            1.5f,
            glm::vec4(0.0f, 0.3f, 1.0f, 1.0f));

        if (m_MainMenu) m_MainMenu->Render(*GetCamera());
        return;
    }

    // Paused screen (render world + pause menu)
    if (m_State == GameState::Paused)
    {
        for (auto& e : m_Enemies) if (e->IsAlive()) e->Render();
        if (m_PlayerBullet) m_PlayerBullet->Render();
        for (auto& b : m_EnemyBullets) b->Render();
        if (m_Player) m_Player->Render();
        if (m_CeilingWall) m_CeilingWall->Render();
        if (m_UFOActive && m_UFO && m_UFO->IsAlive()) m_UFO->Render();

        // Barriers (sheet using correct UV slicing)
        for (size_t i = 0; i < m_Barriers.size(); i++)
        {
            if (!m_Barriers[i]->IsActive()) continue;
            if (m_BarrierHealth[i] <= 0) continue;

            float hp = (float)m_BarrierHealth[i];
            float maxHp = (float)MAX_BARRIER_HEALTH;

            float damage01 = 1.0f - (hp / maxHp);
            damage01 = std::clamp(damage01, 0.0f, 1.0f);

            // Use 5 real frames from your image
            const int FRAMES = 5;
            int frame = (int)std::round(damage01 * (FRAMES - 1));
            frame = std::clamp(frame, 0, FRAMES - 1);

            glm::vec4 uv = GetBarrierFrameUV(frame);
            Renderer::DrawQuadWithTexCoords(
                m_Barriers[i]->GetPosition(),
                m_Barriers[i]->GetSize(),
                m_BarrierSheet.get(),
                uv,
                glm::vec4(1.0f)
            );
        }

        glm::vec2 camPos = GetCamera()->GetPosition();
        TextRenderer::RenderText("PAUSED",
            glm::vec2(camPos.x - 90.0f, camPos.y + 200.0f),
            3.0f,
            glm::vec4(1, 1, 0, 1));

        if (m_PauseMenu) m_PauseMenu->Render(*GetCamera());
        return;
    }

    // Gameplay world
    if (m_CeilingWall) m_CeilingWall->Render();

    if (m_UFOActive && m_UFO && m_UFO->IsAlive())
        m_UFO->Render();

    // Barriers (sheet by health using correct UV slicing)
    for (size_t i = 0; i < m_Barriers.size(); i++)
    {
        if (!m_Barriers[i]->IsActive()) continue;
        if (m_BarrierHealth[i] <= 0) continue;

        float hp = (float)m_BarrierHealth[i];
        float maxHp = (float)MAX_BARRIER_HEALTH;

        float damage01 = 1.0f - (hp / maxHp);
        damage01 = std::clamp(damage01, 0.0f, 1.0f);

        const int FRAMES = 5;
        int frame = (int)std::round(damage01 * (FRAMES - 1));
        frame = std::clamp(frame, 0, FRAMES - 1);

        glm::vec4 uv = GetBarrierFrameUV(frame);

        Renderer::DrawQuadWithTexCoords(
            m_Barriers[i]->GetPosition(),
            m_Barriers[i]->GetSize(),
            m_BarrierSheet.get(),
            uv,
            glm::vec4(1.0f)
        );
    }

    for (auto& e : m_Enemies)
        if (e->IsAlive()) e->Render();

    if (m_PlayerBullet) m_PlayerBullet->Render();
    for (auto& b : m_EnemyBullets) b->Render();
    if (m_Player) m_Player->Render();

    // UI
    glm::vec2 camPos = GetCamera()->GetPosition();

    TextRenderer::RenderText("Score: " + std::to_string(m_Score),
        glm::vec2(camPos.x - 600.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(1, 1, 1, 1));

    TextRenderer::RenderText("FPS: " + std::to_string(Time::GetFPS()),
        glm::vec2(camPos.x - 600.0f, camPos.y + 290.0f),
        1.5f,
        glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

    TextRenderer::RenderText("Lives: " + std::to_string(m_Lives),
        glm::vec2(camPos.x + 450.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(0, 1, 0, 1));

    TextRenderer::RenderText("Level: " + std::to_string(m_Level),
        glm::vec2(camPos.x - 70.0f, camPos.y + 330.0f),
        2.0f,
        glm::vec4(1, 1, 0, 1));

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

    if (m_State == GameState::LevelComplete)
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

void SpaceInvadersGame::OnInput(float deltaTime)
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

void SpaceInvadersGame::NextLevel()
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

void SpaceInvadersGame::RestartGame()
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

void SpaceInvadersGame::GameOver()
{
    m_State = GameState::GameOver;

    AudioManager::PlaySFX("game_over");
    StopGameMusic();
}

// Header declares this, so define it
void SpaceInvadersGame::DrawBackground(Camera* cam, const std::shared_ptr<Texture>& tex)
{
    if (!cam || !tex) return;
    Renderer::DrawQuad(cam->GetPosition(), glm::vec2(1400.0f, 800.0f), tex.get());
}
