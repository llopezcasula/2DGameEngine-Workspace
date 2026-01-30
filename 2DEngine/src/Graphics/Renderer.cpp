#include "Graphics/Renderer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

struct Renderer::RendererData
{
    unsigned int QuadVAO = 0;
    unsigned int QuadVBO = 0;

    std::unique_ptr<Shader> QuadShader;
    glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
};

std::unique_ptr<Renderer::RendererData> Renderer::s_Data = nullptr;

void Renderer::Init()
{
    s_Data = std::make_unique<RendererData>();

    // Quad vertices (position + texture coordinates)
    float vertices[] = {
        // x, y,   u, v
        -0.5f, -0.5f, 0.0f, 0.0f,
         0.5f, -0.5f, 1.0f, 0.0f,
         0.5f,  0.5f, 1.0f, 1.0f,

         0.5f,  0.5f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f
    };

    glGenVertexArrays(1, &s_Data->QuadVAO);
    glGenBuffers(1, &s_Data->QuadVBO);

    glBindVertexArray(s_Data->QuadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, s_Data->QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    // Load shader
    s_Data->QuadShader = std::make_unique<Shader>(
        "assets/shaders/basic.vert",
        "assets/shaders/basic.frag"
    );

    // Set constant uniforms once
    s_Data->QuadShader->Bind();
    s_Data->QuadShader->SetInt("u_Texture", 0);
    s_Data->QuadShader->SetVec2("u_UVMin", glm::vec2(0.0f, 0.0f));
    s_Data->QuadShader->SetVec2("u_UVMax", glm::vec2(1.0f, 1.0f));
    s_Data->QuadShader->Unbind();

    // Enable alpha blending (for PNG transparency)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // If you're doing pure 2D, disable depth test (recommended)
    glDisable(GL_DEPTH_TEST);

    std::cout << "Renderer initialized\n";
}

void Renderer::Shutdown()
{
    if (s_Data)
    {
        glDeleteVertexArrays(1, &s_Data->QuadVAO);
        glDeleteBuffers(1, &s_Data->QuadVBO);
        s_Data.reset();
    }
}

void Renderer::BeginScene(const glm::mat4& viewProjection)
{
    s_Data->ViewProjectionMatrix = viewProjection;

    // Bind once per scene
    s_Data->QuadShader->Bind();
    s_Data->QuadShader->SetMat4("u_ViewProjection", s_Data->ViewProjectionMatrix);
}

void Renderer::EndScene()
{
    // Unbind once per scene
    s_Data->QuadShader->Unbind();
}

void Renderer::Clear(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);

    // For 2D without depth:
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::DrawQuad(const Quad& quad)
{
    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, glm::vec3(quad.position, 0.0f));

    if (quad.rotation != 0.0f)
        transform = glm::rotate(transform, quad.rotation, glm::vec3(0, 0, 1));

    transform = glm::scale(transform, glm::vec3(quad.size, 1.0f));

    // Shader already bound in BeginScene()
    s_Data->QuadShader->SetMat4("u_Transform", transform);
    s_Data->QuadShader->SetVec4("u_Color", quad.color);

    // Full texture UVs
    s_Data->QuadShader->SetVec2("u_UVMin", glm::vec2(0.0f, 0.0f));
    s_Data->QuadShader->SetVec2("u_UVMax", glm::vec2(1.0f, 1.0f));

    if (quad.texture)
    {
        quad.texture->Bind(0);
        s_Data->QuadShader->SetInt("u_UseTexture", 1);
    }
    else
    {
        s_Data->QuadShader->SetInt("u_UseTexture", 0);
    }

    glBindVertexArray(s_Data->QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::DrawQuad(const glm::vec2& position,
                        const glm::vec2& size,
                        const glm::vec4& color)
{
    DrawQuad(Quad(position, size, color, 0.0f, nullptr));
}

void Renderer::DrawQuad(const glm::vec2& position,
                        const glm::vec2& size,
                        Texture* texture,
                        const glm::vec4& tint)
{
    DrawQuad(Quad(position, size, tint, 0.0f, texture));
}

void Renderer::DrawQuadWithTexCoords(const glm::vec2& position,
                                     const glm::vec2& size,
                                     Texture* texture,
                                     const glm::vec4& texCoords,
                                     const glm::vec4& tint)
{
    glm::mat4 transform(1.0f);
    transform = glm::translate(transform, glm::vec3(position, 0.0f));
    transform = glm::scale(transform, glm::vec3(size, 1.0f));

    // Shader already bound in BeginScene()
    s_Data->QuadShader->SetMat4("u_Transform", transform);
    s_Data->QuadShader->SetVec4("u_Color", tint);

    // Sprite sheet UVs via uniforms (FAST)
    s_Data->QuadShader->SetVec2("u_UVMin", glm::vec2(texCoords.x, texCoords.y));
    s_Data->QuadShader->SetVec2("u_UVMax", glm::vec2(texCoords.z, texCoords.w));

    if (texture)
    {
        texture->Bind(0);
        s_Data->QuadShader->SetInt("u_UseTexture", 1);
    }
    else
    {
        s_Data->QuadShader->SetInt("u_UseTexture", 0);
    }

    glBindVertexArray(s_Data->QuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
