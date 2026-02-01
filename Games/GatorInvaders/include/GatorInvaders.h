#pragma once

#include "Core/Game.h"
#include "Audio/AudioManager.h"
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <glm/glm.hpp>

// Forward declarations
class Player;
class Enemy;
class Bullet;
class Obstacle;
class Entity;
class Menu;
class Time;
class Texture;

enum class GameState
{
    MainMenu,
    Controls,
    Leaderboard,
    EnterInitials,
    Playing,
    Paused,
    GameOver,
    LevelComplete,
    PlayerHit
};

class GatorInvaders : public Game
{
public:
    GatorInvaders();
    ~GatorInvaders();

    // Override Game lifecycle methods
    void OnInit() override;
    void OnShutdown() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnInput(float deltaTime) override;

private:
    // Game flow
    void SpawnEnemyGrid();
    void SpawnBarriers();
    void ShootBullet();
    void EnemyShoot();
    void CheckCollisions();
    void CleanupDeadEntities();
    void NextLevel();
    void RestartGame();
    void GameOver();

    // Menu handling
    void SetupMainMenu();
    void SetupPauseMenu();
    void ShowMainMenu();
    void ShowPauseMenu();
    void StartGame();
    void QuitGame();
    void TogglePause();

    // Audio handling
    void InitAudio();
    void LoadGameSounds();
    void PlayGameMusic();
    void StopGameMusic();

    // Texture loading
    void LoadTextures();
    static void DrawBackground(Camera* cam, const std::shared_ptr<Texture>& tex);

    // Game State
    GameState m_State;
    bool m_AudioInitialized;
    bool m_ControlsReturnToPause = false;
    bool m_LeaderboardReturnToPause = false;

    // Menus
    std::unique_ptr<Menu> m_MainMenu;
    std::unique_ptr<Menu> m_PauseMenu;
    std::unique_ptr<Menu> m_ControlsMenu;
    std::unique_ptr<Menu> m_LeaderboardMenu;


    // Player
    std::unique_ptr<Player> m_Player;
    glm::vec2 m_PlayerStartPos;
    float m_PlayerSpeed;
    float m_PlayerMoveRange;

    // Enemies
    std::vector<std::unique_ptr<Enemy>> m_Enemies;
    std::vector<int> m_EnemyRowScores;
    int m_EnemyRows;
    int m_EnemyColumns;
    float m_EnemyMoveTimer;
    float m_EnemyMoveInterval;
    float m_BaseMoveInterval;
    float m_EnemyMoveDistance;
    int m_EnemyDirection;
    bool m_EnemiesShouldMoveDown;
    int m_CurrentEnemyFrame;  // For animation (0 or 1)

    // Bullets
    std::unique_ptr<Bullet> m_PlayerBullet;
    std::vector<std::unique_ptr<Bullet>> m_EnemyBullets;
    float m_EnemyShootTimer;
    float m_EnemyShootInterval;
    float m_PlayerShootCooldown;
    float m_PlayerShootTimer;
    static constexpr float PLAYER_SHOOT_RATE = 0.4f;

    // Ceiling wall
    std::unique_ptr<Obstacle> m_CeilingWall;
    float m_CeilingY;

    // Barriers - Classic Space Invaders style (4x3 grid, bottom row only has 2 outer parts)
    static constexpr int BARRIER_COUNT = 4;
    static constexpr int BARRIER_PARTS = 10;      // 4 + 4 + 2 = 10 parts
    static constexpr int BARRIER_COLS = 4;        // 4 columns wide
    static constexpr int BARRIER_ROWS = 3;        // 3 rows high
    static constexpr int BARRIER_STAGES = 4;      // barrier0..barrier3

    struct BarrierPart
    {
        std::unique_ptr<Obstacle> obstacle; // collider owner
        int stage = 0;                      // 0..3
        bool broken = false;                // true => bullets pass through
        int gridRow = 0;                    // 0-3 (top to bottom)
        int gridCol = 0;                    // 0-2 (left to right)
    };

    struct Barrier
    {
        glm::vec2 center;
        glm::vec2 size;
        std::array<BarrierPart, BARRIER_PARTS> parts;
    };

    std::array<std::unique_ptr<Texture>, BARRIER_STAGES> m_BarrierStageTextures;
    std::array<Barrier, BARRIER_COUNT> m_BarriersSplit;

    // Quick lookup from collider owner -> (barrierIndex, partIndex)
    std::unordered_map<Entity*, std::pair<int,int>> m_BarrierPartLookup;

    // UFO
    std::unique_ptr<Enemy> m_UFO;
    bool m_UFOActive;
    float m_UFOSpawnTimer;
    float m_UFOSpawnInterval;
    int m_UFODirection;

    // Game state
    int m_Score;
    int m_Lives;
    int m_Level;

    // Player hit
    float m_PlayerHitTimer;
    static constexpr float PLAYER_HIT_FREEZE_DURATION = 1.0f;

    // Textures - Using sprite sheets (2 frames per texture file)
    std::unique_ptr<Texture> m_PlayerTexture;
    std::unique_ptr<Texture> m_Enemy1Texture;  // Sprite sheet with 2 frames
    std::unique_ptr<Texture> m_Enemy2Texture;  // Sprite sheet with 2 frames
    std::unique_ptr<Texture> m_Enemy3Texture;  // Sprite sheet with 2 frames
    std::unique_ptr<Texture> m_UFOTexture;     // Sprite sheet with 2 frames
    std::unique_ptr<Texture> m_GatorAlienTexture; // 2-frame sheet (closed/open)
    std::shared_ptr<Texture> m_BackgroundTexture;

    void SetupControlsMenu();
    void ShowControlsMenu(bool returnToPause);
    void CloseControlsMenu();

    struct LeaderboardEntry
    {
        std::string initials; // 3 letters
        int score = 0;
    };

    std::vector<LeaderboardEntry> m_Leaderboard; // sorted desc, max 10

    std::string m_InitialsInput;   // typed initials
    int m_PendingScore = 0;        // score being submitted


    // Leaderboard
    void SetupLeaderboardMenu();
    void ShowLeaderboardMenu(bool returnToPause);
    void CloseLeaderboardMenu();

    void LoadLeaderboard();
    void SaveLeaderboard();
    bool IsHighScore(int score) const;
    void BeginInitialsEntry();
    void SubmitInitialsEntry();


};