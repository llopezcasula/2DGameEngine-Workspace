#include "Core/Window.h"
#include <GLFW/glfw3.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Window::Window(const WindowProps& props)
    : m_Title(props.Title)
    , m_Width(props.Width)
    , m_Height(props.Height)
    , m_VSync(props.VSync)
    , m_IsFullscreen(props.Fullscreen)
    , m_WindowedX(100)
    , m_WindowedY(100)
    , m_WindowedWidth(props.Width)
    , m_WindowedHeight(props.Height)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWmonitor* monitor = m_IsFullscreen ? glfwGetPrimaryMonitor() : nullptr;
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), monitor, nullptr);

    if (!m_Window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);
    SetVSync(m_VSync);

    std::cout << "Window created: " << m_Width << "x" << m_Height << "\n";
}

Window::~Window()
{
    if (m_Window)
    {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}

void Window::SetFullscreen(bool fullscreen)
{
    if (m_IsFullscreen == fullscreen) return;

    m_IsFullscreen = fullscreen;

    if (m_IsFullscreen)
    {
        // Save windowed position and size
        glfwGetWindowPos(m_Window, &m_WindowedX, &m_WindowedY);
        glfwGetWindowSize(m_Window, &m_WindowedWidth, &m_WindowedHeight);

        // Get primary monitor and its video mode
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // Switch to fullscreen
        glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

        m_Width = mode->width;
        m_Height = mode->height;
    }
    else
    {
        // Restore windowed mode
        glfwSetWindowMonitor(m_Window, nullptr, m_WindowedX, m_WindowedY,
                            m_WindowedWidth, m_WindowedHeight, GLFW_DONT_CARE);

        m_Width = m_WindowedWidth;
        m_Height = m_WindowedHeight;
    }
}

void Window::SetVSync(bool enabled)
{
    m_VSync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

void Window::SetIcon(const std::string& iconPath)
{
    GLFWimage image{};
    int channels = 0;

    stbi_set_flip_vertically_on_load(false); // icons usually should NOT flip
    image.pixels = stbi_load(iconPath.c_str(), &image.width, &image.height, &channels, 4);

    if (!image.pixels)
    {
        std::cerr << "Failed to load window icon: " << iconPath << "\n";
        return;
    }

    glfwSetWindowIcon(m_Window, 1, &image);
    stbi_image_free(image.pixels);

    std::cout << "Window icon set: " << iconPath << "\n";
}

void Window::SetTitle(const std::string& title)
{
    m_Title = title;
    glfwSetWindowTitle(m_Window, title.c_str());
}

bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_Window);
}