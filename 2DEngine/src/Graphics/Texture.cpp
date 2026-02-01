#include "Graphics/Texture.h"
#include <glad/glad.h>

#include <stb_image.h>
#include <iostream>

Texture::Texture(const std::string& path)
    : m_RendererID(0), m_Path(path), m_Width(0), m_Height(0), m_Channels(0)
{
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &m_Width, &m_Height, &m_Channels, 0);

    if (!data)
    {
        std::cerr << "Failed to load texture: " << path << "\n";
        return;
    }

    GLenum internalFormat = 0, dataFormat = 0;
    if (m_Channels == 4)
    {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    }
    else if (m_Channels == 3)
    {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    }
    else
    {
        std::cerr << "Unsupported channel count (" << m_Channels << ") for texture: " << path << "\n";
        stbi_image_free(data);
        return;
    }

    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    // Pixel-art friendly filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, GL_UNSIGNED_BYTE, data);

    // No mipmaps for pixel sprites (faster + avoids blur/shimmer)
    // glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "Loaded texture: " << path << " (" << m_Width << "x" << m_Height << ")\n";
}

Texture::Texture(unsigned int existingID, int width, int height)
    : m_RendererID(existingID)
    , m_Path("")
    , m_Width(width)
    , m_Height(height)
    , m_Channels(4)
    , m_OwnsGLTexture(false) {
}

Texture::~Texture()
{
    if (m_OwnsGLTexture && m_RendererID != 0)
        glDeleteTextures(1, &m_RendererID);
}

void Texture::Bind(unsigned int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

void Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}
