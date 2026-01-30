#pragma once

#include "Core/Game.h"
#include "Audio/AudioManager.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

// Forward declarations
class Player;
class Enemy;
class Bullet;
class Obstacle;
class Menu;
class Time;
class Texture;

enum class GameState
{
    MainMenu,
    Playing,
    Paused,
    GameOver,
    LevelComplete,
    PlayerHit
};

class SpaceInvadersGame : public Game
{
public:
    SpaceInvadersGame();
    ~SpaceInvadersGame();

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
    void UpdateBarrierColors();
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

    // Menus
    std::unique_ptr<Menu> m_MainMenu;
    std::unique_ptr<Menu> m_PauseMenu;

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

    // Barriers
    std::vector<std::unique_ptr<Obstacle>> m_Barriers;
    std::vector<int> m_BarrierHealth;
    static constexpr int MAX_BARRIER_HEALTH = 20;

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
    static constexpr int BARRIER_FRAMES = 10;

    std::unique_ptr<Texture> m_BarrierSheet;

};