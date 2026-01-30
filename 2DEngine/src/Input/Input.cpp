#include "Input/Input.h"
#include <GLFW/glfw3.h>
#include "Graphics/Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>

GLFWwindow* Input::s_Window = nullptr;
bool Input::s_Keys[KEY_LAST + 1] = {};
bool Input::s_MouseButtons[MOUSE_BUTTON_LAST + 1] = {};
bool Input::s_KeysPrev[KEY_LAST + 1] = {};
bool Input::s_MouseButtonsPrev[MOUSE_BUTTON_LAST + 1] = {};
glm::vec2 Input::s_MousePosition = glm::vec2(0.0f);
glm::vec2 Input::s_MouseScroll = glm::vec2(0.0f);

void Input::Init(GLFWwindow* window)
{
    s_Window = window;

    // Set up callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
}

void Input::Update()
{
    // Copy current state to previous state
    std::memcpy(s_KeysPrev, s_Keys, sizeof(s_Keys));
    std::memcpy(s_MouseButtonsPrev, s_MouseButtons, sizeof(s_MouseButtons));

    // Reset scroll (it's per-frame)
    s_MouseScroll = glm::vec2(0.0f);
}

bool Input::IsKeyPressed(int key)
{
    if (key < 0 || key > KEY_LAST) return false;
    return s_Keys[key];
}

bool Input::IsKeyJustPressed(int key)
{
    if (key < 0 || key > KEY_LAST) return false;
    return s_Keys[key] && !s_KeysPrev[key];
}

bool Input::IsKeyJustReleased(int key)
{
    if (key < 0 || key > KEY_LAST) return false;
    return !s_Keys[key] && s_KeysPrev[key];
}

bool Input::IsMouseButtonPressed(int button)
{
    if (button < 0 || button > MOUSE_BUTTON_LAST) return false;
    return s_MouseButtons[button];
}

bool Input::IsMouseButtonJustPressed(int button)
{
    if (button < 0 || button > MOUSE_BUTTON_LAST) return false;
    return s_MouseButtons[button] && !s_MouseButtonsPrev[button];
}

bool Input::IsMouseButtonJustReleased(int button)
{
    if (button < 0 || button > MOUSE_BUTTON_LAST) return false;
    return !s_MouseButtons[button] && s_MouseButtonsPrev[button];
}

glm::vec2 Input::GetMousePosition()
{
    return s_MousePosition;
}

glm::vec2 Input::GetMouseWorldPosition(const Camera& camera)
{
    // Get window size
    int width, height;
    glfwGetWindowSize(s_Window, &width, &height);

    // Convert screen coordinates to NDC (-1 to 1)
    glm::vec2 ndc;
    ndc.x = (2.0f * s_MousePosition.x) / width - 1.0f;
    ndc.y = 1.0f - (2.0f * s_MousePosition.y) / height; // Flip Y

    // Convert NDC to world space using inverse view-projection matrix
    glm::mat4 invVP = glm::inverse(camera.GetViewProjectionMatrix());
    glm::vec4 worldPos = invVP * glm::vec4(ndc, 0.0f, 1.0f);

    return glm::vec2(worldPos.x, worldPos.y);
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key < 0 || key > KEY_LAST) return;

    if (action == GLFW_PRESS)
        s_Keys[key] = true;
    else if (action == GLFW_RELEASE)
        s_Keys[key] = false;
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button < 0 || button > MOUSE_BUTTON_LAST) return;
    
    if (action == GLFW_PRESS)
        s_MouseButtons[button] = true;
    else if (action == GLFW_RELEASE)
        s_MouseButtons[button] = false;
}

void Input::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    s_MousePosition = glm::vec2((float)xpos, (float)ypos);
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    s_MouseScroll = glm::vec2((float)xoffset, (float)yoffset);
}