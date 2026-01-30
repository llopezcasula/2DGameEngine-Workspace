#pragma once

#include <string>

// Forward declaration to avoid including GLFW in header
struct GLFWwindow;

struct WindowProps
{
    std::string Title;
    unsigned int Width;
    unsigned int Height;
    bool VSync;
    bool Fullscreen;

    WindowProps(const std::string& title = "2D Engine",
                unsigned int width = 800,
                unsigned int height = 600,
                bool vsync = true,
                bool fullscreen = false)
        : Title(title), Width(width), Height(height), VSync(vsync), Fullscreen(fullscreen)
    {
    }
};

class Window
{
public:
    Window(const WindowProps& props);
    ~Window();

    GLFWwindow* GetNativeWindow() const { return m_Window; }

    unsigned int GetWidth() const { return m_Width; }
    unsigned int GetHeight() const { return m_Height; }

    bool IsFullscreen() const { return m_IsFullscreen; }
    void SetFullscreen(bool fullscreen);
    void ToggleFullscreen() { SetFullscreen(!m_IsFullscreen); }

    void SetVSync(bool enabled);
    bool IsVSync() const { return m_VSync; }

    void SetIcon(const std::string& iconPath);
    void SetTitle(const std::string& title);

    bool ShouldClose() const;

private:
    GLFWwindow* m_Window;
    std::string m_Title;
    unsigned int m_Width;
    unsigned int m_Height;
    bool m_VSync;
    bool m_IsFullscreen;
    
    // Store windowed mode properties for fullscreen toggle
    int m_WindowedX, m_WindowedY;
    int m_WindowedWidth, m_WindowedHeight;
};