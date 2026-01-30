#pragma once

#include <glm/glm.hpp>
#include "Input/Keys.h"

// Forward declarations to avoid including GLFW in header
struct GLFWwindow;

// Static input manager - call Update() once per frame
class Input
{
public:
    static void Init(GLFWwindow* window);
    static void Update();

    // Keyboard
    static bool IsKeyPressed(int key);
    static bool IsKeyJustPressed(int key);
    static bool IsKeyJustReleased(int key);

    // Mouse buttons
    static bool IsMouseButtonPressed(int button);
    static bool IsMouseButtonJustPressed(int button);
    static bool IsMouseButtonJustReleased(int button);

    // Mouse position (screen coordinates)
    static glm::vec2 GetMousePosition();
    
    // Mouse position (world coordinates, requires camera)
    static glm::vec2 GetMouseWorldPosition(const class Camera& camera);

    // Mouse scroll
    static glm::vec2 GetMouseScroll() { return s_MouseScroll; }

private:
    static GLFWwindow* s_Window;
    
    // Current frame state
    static bool s_Keys[KEY_LAST + 1];
    static bool s_MouseButtons[MOUSE_BUTTON_LAST + 1];

    // Previous frame state (for "just pressed" detection)
    static bool s_KeysPrev[KEY_LAST + 1];
    static bool s_MouseButtonsPrev[MOUSE_BUTTON_LAST + 1];
    
    static glm::vec2 s_MousePosition;
    static glm::vec2 s_MouseScroll;

    // GLFW callbacks
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};