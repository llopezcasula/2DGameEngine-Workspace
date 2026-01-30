#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

uniform vec2 u_UVMin;
uniform vec2 u_UVMax;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = mix(u_UVMin, u_UVMax, a_TexCoord);
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 0.0, 1.0);
}
