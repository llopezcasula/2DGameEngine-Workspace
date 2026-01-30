#pragma once

#include <memory>

class Game;
class Window;
class Camera;

// The main engine runner
class Engine
{
public:
    Engine();
    ~Engine();

    void Run(Game* game);

private:
    void Init();
    void Shutdown();
    void GameLoop();

    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Camera> m_Camera;
    Game* m_CurrentGame;
};