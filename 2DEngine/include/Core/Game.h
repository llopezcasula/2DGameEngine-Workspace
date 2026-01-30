#pragma once

#include <memory>

class Window;
class Camera;

// Base class that all games inherit from
class Game
{
public:
    virtual ~Game() = default;

    // Game lifecycle - override these in your games
    virtual void OnInit() = 0;
    virtual void OnShutdown() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
    virtual void OnFixedUpdate(float fixedDeltaTime) {}
    virtual void OnRender() = 0;
    virtual void OnInput(float deltaTime) {}

    // Engine access
    void SetWindow(Window* window) { m_Window = window; }
    void SetCamera(Camera* camera) { m_Camera = camera; }
    
    Window* GetWindow() { return m_Window; }
    Camera* GetCamera() { return m_Camera; }

protected:
    Window* m_Window = nullptr;
    Camera* m_Camera = nullptr;
};