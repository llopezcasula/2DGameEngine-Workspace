#include "Core/Engine.h"
#include "Core/Game.h"
#include "Core/Window.h"
#include "Core/Time.h"
#include "Graphics/Camera.h"
#include "Graphics/Renderer.h"
#include "Graphics/TextRenderer.h"
#include "Audio/AudioManager.h"
#include "Input/Input.h"
#include "Physics/Physics.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Engine::Engine()
    : m_CurrentGame(nullptr)
{
}

Engine::~Engine()
{
    Shutdown();
}

void Engine::Init()
{
    // Create window
    WindowProps props;
    props.Title = "2D Game Engine";
    props.Width = 1280;
    props.Height = 720;
    props.VSync = true;
    props.Fullscreen = false;

    m_Window = std::make_unique<Window>(props);

    glfwSetFramebufferSizeCallback(m_Window->GetNativeWindow(), FramebufferSizeCallback);

    // Load OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return;
    }

    glViewport(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

    std::cout << "=========================\n";
    std::cout << "2D Game Engine v1.0\n";
    std::cout << "=========================\n";
    std::cout << "OpenGL " << glGetString(GL_VERSION) << "\n";
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "=========================\n\n";

    // Initialize all engine systems
    Time::Init();
    Renderer::Init();
    TextRenderer::Init();
    AudioManager::Init();
    Input::Init(m_Window->GetNativeWindow());
    Physics::Init();

    // Create camera
    m_Camera = std::make_unique<Camera>(
        (float)m_Window->GetWidth(),
        (float)m_Window->GetHeight()
    );
}

void Engine::Run(Game* game)
{
    if (!game)
    {
        std::cerr << "ERROR: No game provided to engine!\n";
        return;
    }

    m_CurrentGame = game;

    Init();

    // Give game access to engine systems
    m_CurrentGame->SetWindow(m_Window.get());
    m_CurrentGame->SetCamera(m_Camera.get());

    std::cout << "Initializing game...\n";
    m_CurrentGame->OnInit();
    std::cout << "Game initialized!\n\n";

    // Run game loop
    GameLoop();

    // Cleanup
    std::cout << "\nShutting down game...\n";
    m_CurrentGame->OnShutdown();
}

void Engine::GameLoop()
{
    std::cout << "Starting game loop...\n";

    while (!m_Window->ShouldClose())
    {
        Time::Update();
        float deltaTime = Time::DeltaTime();

        Input::Update();
        glfwPollEvents();

        // Game input
        m_CurrentGame->OnInput(deltaTime);

        // Fixed timestep for physics
        Time::AddToAccumulator(deltaTime);
        while (Time::GetAccumulator() >= Time::FixedDeltaTime())
        {
            m_CurrentGame->OnFixedUpdate(Time::FixedDeltaTime());
            Time::ReduceAccumulator();
        }

        // Update game
        m_CurrentGame->OnUpdate(deltaTime);

        // Render
        Renderer::Clear(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        Renderer::BeginScene(m_Camera->GetViewProjectionMatrix());

        m_CurrentGame->OnRender();

        Physics::DebugRenderColliders();

        Renderer::EndScene();

        glfwSwapBuffers(m_Window->GetNativeWindow());
    }
}

void Engine::Shutdown()
{
    m_Camera.reset();

    Physics::Shutdown();
    AudioManager::Shutdown();
    TextRenderer::Shutdown();
    Renderer::Shutdown();

    m_Window.reset();

    std::cout << "Engine shut down.\n";
}