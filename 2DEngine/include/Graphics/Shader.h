#pragma once

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    // Constructor reads and builds the shader from file paths
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Use/activate the shader
    void Bind() const;
    void Unbind() const;

    // Utility uniform functions
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

    unsigned int GetID() const { return m_RendererID; }

private:
    unsigned int m_RendererID;

    // Utility function for checking shader compilation/linking errors
    void CheckCompileErrors(unsigned int shader, const std::string& type);
};

