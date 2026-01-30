#pragma once

#include <glm/glm.hpp>
#include <string>
#include <map>
#include <memory>

class Shader;
class Texture;

struct Character
{
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class TextRenderer
{
public:
    static void Init();
    static void Shutdown();

    // Load a font from file (requires FreeType)
    static bool LoadFont(const std::string& fontPath, unsigned int fontSize);

    // Render text at position (screen coordinates or world coordinates depending on camera)
    static void RenderText(const std::string& text, 
                          glm::vec2 position, 
                          float scale = 1.0f,
                          glm::vec4 color = glm::vec4(1.0f));

    // Get text width for layout calculations
    static float GetTextWidth(const std::string& text, float scale = 1.0f);

private:
    struct TextData;
    static std::unique_ptr<TextData> s_Data;
};