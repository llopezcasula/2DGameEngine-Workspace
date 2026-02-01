#pragma once
#include <string>

class Texture
{
public:
    Texture(const std::string& path);

    // NEW: Wrap an existing OpenGL texture ID (does NOT delete it)
    Texture(unsigned int existingID, int width, int height);

    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    unsigned int GetID() const { return m_RendererID; }

private:
    unsigned int m_RendererID;
    std::string m_Path;
    int m_Width, m_Height, m_Channels;

    // NEW:
    bool m_OwnsGLTexture = true;
};
